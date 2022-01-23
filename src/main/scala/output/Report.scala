package output

import ikuy_utils._
import overlord.Instances.{BoardInstance, ChipInstance, Container, InstanceTrait}
import overlord._
import scalax.collection.Graph
import scalax.collection.GraphEdge.DiEdge
import scalax.collection.GraphPredef.EdgeAssoc
import scalax.collection.io.dot.implicits._
import scalax.collection.io.dot.{DotGraph, _}

import java.nio.file.Path
import scala.collection.mutable

object Report {
	private type GraphType = Graph[(String, DefinitionType, NodeStyleTypeTag), DiEdge]
	private type EdgeType = GraphType#EdgeT
	private type EdgeResult = Option[(DotGraph, DotEdgeStmt)]
	private type NodeType = GraphType#NodeT
	private type NodeResult = Option[(DotGraph, DotNodeStmt)]
	private val cpuDotAttribs      = Seq(DotAttr("shape", "diamond"),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("color", "white"),
	                                     DotAttr("fillcolor", "green"))
	private val ioDotAttribs       = Seq(DotAttr("shape", "box"),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("color", "white"),
	                                     DotAttr("fillcolor", "lightblue"))
	private val ramDotAttribs      = Seq(DotAttr("shape", "box"),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("color", "white"),
	                                     DotAttr("fillcolor", "tomato"))
	private val storageDotAttribs  = Seq(DotAttr("shape", "box"),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("color", "white"),
	                                     DotAttr("fillcolor", "palevioletred"))
	private val busDotAttribs      = Seq(DotAttr("shape", "oval"),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("color", "white"),
	                                     DotAttr("fillcolor", "darkolivegreen"))
	private val bridgeDotAttribs   = Seq(DotAttr("label", "Bridge"),
	                                     DotAttr("shape", "invtriangle"),
	                                     DotAttr("fontsize", 6),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("color", "white"),
	                                     DotAttr("fillcolor", "whitesmoke"))
	private val pinGroupDotAttribs = Seq(DotAttr("shape", "oval"),
	                                     DotAttr("style", "filled"),
	                                     DotAttr("fontcolor", "white"),
	                                     DotAttr("fillcolor", "darkblue"))

	private val defaultDotAttribs     = Seq(DotAttr("shape", "box"))
	private val ramEdgeDotAttribs     = Seq(DotAttr("style", "dashed"))
	private val busEdgeDotAttribs     = Seq(DotAttr("color", "red"))
	private val bridgeEdgeDotAttribs  = Seq(DotAttr("style", "dashed"))
	private val defaultEdgeDotAttribs = Seq(DotAttr("color", "black"))

	def apply(game: Game, out: Path): Unit = {

		val sb   = new StringBuilder
		val cpus = game.cpus

		val cpuTypes = cpus.map(_.definition).toSet
		sb ++= f"------------------%n"
		sb ++= f"${cpus.map(_.cpuCount).sum} CPU cores of ${cpuTypes.size} different types%n"
		sb ++= f"------------------%n"
		for (cput <- cpuTypes) {
			val chipType = cput.defType.ident.mkString
			val arch     = Utils.lookupString(cput.attributes, "arch", "UNKNOWN")
			val bw       = Utils.lookupInt(cput.attributes, "width", 32)

			val coreCount = cpus.filter(_.definition == cput).map(_.cpuCount).sum

			sb ++= f"$coreCount cores of $chipType ($bw bit $arch) CPU%n"
		}

		sb ++= f"%n------------------%n"
		sb ++= f"Instances%n"
		sb ++= f"------------------%n%n"

		sb ++= reportInstances(game)

		sb ++= f"%n------------------%n"
		sb ++= f"Connections%n"
		sb ++= f"------------------%n%n"
		for (connection <- game.connected) {
			sb ++= f"------------------%n"
			sb ++= f"${connection.firstFullName} <> ${connection.secondFullName}%n"
		}

		val boardIndex = game.distanceMatrix
			.instanceArray
			.indexWhere(_.isInstanceOf[BoardInstance])
		sb ++= f"%n------------------%n"
		sb ++= f"Board Index = $boardIndex%n"
		sb ++= f"------------------%n"

		// solve for board connected pins
		val boardConnectedPins = mutable.HashMap[Int, Int]()
		for {connected <- game.connected
		     if connected.isPinToChip
		     (sp, ep) = game.distanceMatrix.indicesOf(connected)
		     route = game.distanceMatrix.routeBetween(sp, ep)
		     if route.contains(boardIndex)
		     } boardConnectedPins += (sp -> ep)

		boardConnectedPins.foreach(
			b => {
				val first  = game.distanceMatrix.instanceArray(b._1)
				val second = game.distanceMatrix.instanceArray(b._2)
				sb ++= f" ${first.ident} ${second.ident}%n"
			}
			)

		sb ++= game.distanceMatrix.toString

		Utils.writeFile(out.resolve("report.txt"), sb.result())

		outputDotGraph(game, out)
	}

	private def reportInstances(game: Game): String = {
		game.children.map(reportInstance(_)).mkString("")
	}

	private def outputDotGraph(game: Game, out: Path) = {
		var (graph, maxDistance) = convertToGraph(game)

		graph = makeLegendGraph(graph, maxDistance)

		val root   = DotRootGraph(directed = true,
		                          id = Some(s"${game.name}"),
		                          attrStmts = Seq(
			                          DotAttrStmt(Elem.graph, Seq(DotAttr("ranksep", 0.02))),
			                          DotAttrStmt(Elem.edge, Seq(DotAttr("minlen", 3)))
			                          )
		                          )
		val legend = DotSubGraph(ancestor = root, subgraphId = "cluster_Legend")
		val pins   = DotSubGraph(ancestor = root, subgraphId = "pins",
		                         attrStmts = Seq(
			                         DotAttrStmt(Elem.graph, Seq(DotAttr("rank", "max"))),
			                         ))

		val context = GraphVizContent(Map(
			("root" -> root),
			("legend" -> legend),
			("pins" -> pins)
			))

		val dotText = graph.toDot(
			dotRoot = root,
			edgeTransformer = context.edgePrep,
			cNodeTransformer = Some(context.nodePrep),
			)

		Utils.writeFile(out.resolve(s"${game.name}.gv"), dotText)
	}

	private def makeLegendGraph(graph: GraphType, maxDistance: Int): GraphType = {

		val classCount                    = Utils
			.KnownSubClassesOfSealedType[DefinitionType]()
			.size
		// title
		var dinsts: Array[DefinitionType] = Array(BoardDefinitionType(Seq(s"Legend")))

		// each class
		dinsts ++= Utils.KnownSubClassesOfSealedType[DefinitionType]().map {
			clazz =>
				Utils.ConstructFromClassSymbol(clazz, clazz.name).asInstanceOf[DefinitionType]
		}

		if (classCount < maxDistance * 3) {
			// some spacers
			dinsts ++= {
				0 until (maxDistance * 3 - classCount) map {
					i => BoardDefinitionType(Seq(s"spacer$i"))
				}
			}
		}

		var g = graph
		for (index <- 0 until dinsts.length - 1) {
			val sinst = dinsts(index)
			val einst = dinsts(index + 1)

			def indexToTag(index: Int) =
				index match {
					case 0                               => TitleTypeTag
					case x if 1 to classCount contains x => LegendTypeTag
					case _                               => InvisibleSpacerTypeTag
				}

			val si = sinst.ident.mkString.replace("DefinitionType", "")
			val ei = einst.ident.mkString.replace("DefinitionType", "")

			g += ((si, sinst, indexToTag(index))
			      ~>
			      (ei, einst, indexToTag(index + 1)))
		}
		g
	}

	private def convertToGraph(game: Game) = {
		var graph: GraphType = Graph[(String, DefinitionType, NodeStyleTypeTag), DiEdge]()
		for (node <- game.distanceMatrix.instanceArray) {
			graph += ((node.ident, node.definition.defType, ChipTypeTag))
		}

		var maxDistance = 0

		for {i <- 0 until game.distanceMatrix.dim
		     if !game.distanceMatrix.isIsolated(i)} {
			for {j <- 0 until game.distanceMatrix.dim
			     if !game.distanceMatrix.isIsolated(j)} {
				maxDistance = maxDistance.max(game.distanceMatrix.distanceBetween(i, j))

				val route      = game.distanceMatrix.routeBetween(i, j)
				var startIndex = i
				for {endIndex <- route.indices} {
					val sinst = game.distanceMatrix.instanceArray(startIndex)
					val einst = game.distanceMatrix.instanceArray(route(endIndex))
					graph += ((sinst.ident, sinst.definition.defType, ChipTypeTag)
					          ~>
					          (einst.ident, einst.definition.defType, ChipTypeTag))
					startIndex = route(endIndex)
				}
			}
		}
		(graph, maxDistance)
	}

	private def reportContainer(container: Container,
	                            indentLevel: Int = 0): String = {
		container.children.map(reportInstance(_, indentLevel)).mkString("")
	}

	private def reportInstance(instance: InstanceTrait,
	                           indentLevel: Int = 0): String = {
		val sb = new StringBuilder

		val indent = "\t" * indentLevel
		sb ++= (indent + f"------------------%n")
		sb ++= (indent + instance.ident + f"%n")
		val id   = instance.definition.defType.ident.mkString(".")
		val name = (instance.definition.defType match {
			case RamDefinitionType(ident)      => "ram."
			case CpuDefinitionType(ident)      => "cpu."
			case BusDefinitionType(ident)      => "bus."
			case StorageDefinitionType(ident)  => "storage."
			case BridgeDefinitionType(ident)   => "bridge."
			case NetDefinitionType(ident)      => "net."
			case IoDefinitionType(ident)       => "io."
			case OtherDefinitionType(ident)    => "other."
			case PinGroupDefinitionType(ident) => "pin."
			case ClockDefinitionType(ident)    => "clock."
			case BoardDefinitionType(ident)    => "board."
			case ProgramDefinitionType(ident)  => "program."
			case LibraryDefinitionType(ident)  => "library."
		}) + id
		sb ++= (indent + f"type: $name%n")
		instance match {
			case ci: ChipInstance =>
				for (rl <- ci.registerLists)
					sb ++= f"   ${rl.name} - ${rl.description}%n"
			case _                =>
		}
		instance match {
			case c: Container =>
				sb ++= reportContainer(c, indentLevel + 1)
			case _            =>
		}

		sb.result()
	}

	private sealed class NodeStyleTypeTag

	case class GraphVizContent(graphs: Map[String, DotGraph]) {
		def edgePrep(innerEdge: EdgeType): EdgeResult = {
			val root = graphs("root")

			def attributes(edef: DefinitionType,
			               typeTag: NodeStyleTypeTag) = {
				{
					edef match {
						case RamDefinitionType(ident)    => ramEdgeDotAttribs
						case BusDefinitionType(ident)    => busEdgeDotAttribs
						case BridgeDefinitionType(ident) => bridgeEdgeDotAttribs
						case _                           => defaultEdgeDotAttribs
					}
				} ++ {
					typeTag match {
						case TitleTypeTag | LegendTypeTag | InvisibleSpacerTypeTag =>
							Seq(
								DotAttr("style", "invis"),
								DotAttr("minlen", 1)
								)
						case ChipTypeTag                                           =>
							Seq()
						case _                                                     =>
							println(s"Unknown Graph Node Type Tag ${typeTag}")
							Seq()
					}
				}
			}

			innerEdge.edge match {
				case DiEdge(source, target) => {
					val (sident, sdef, _)       = source.value
					val (eident, edef, typeTag) = target.value
					val dattribs                = attributes(edef, typeTag)
					Some((root, DotEdgeStmt(sident, eident, dattribs)))
				}
			}
		}

		def nodePrep(innerNode: NodeType): NodeResult = {
			val (ident, defType, typeTag) = innerNode.value

			val root   = graphs("root")
			val legend = graphs("legend")
			val pins   = graphs("pins")

			var graph: DotGraph = root

			var dattribs = defType match {
				case CpuDefinitionType(_)      => cpuDotAttribs
				case IoDefinitionType(_)       => ioDotAttribs
				case RamDefinitionType(_)      => ramDotAttribs
				case StorageDefinitionType(_)  => storageDotAttribs
				case BusDefinitionType(_)      => busDotAttribs
				case BridgeDefinitionType(_)   => bridgeDotAttribs
				case PinGroupDefinitionType(_) => graph = pins; pinGroupDotAttribs
				case _                         => defaultDotAttribs
			}
			dattribs ++= (typeTag match {
				case TitleTypeTag           =>
					graph = legend
					Seq(
						DotAttr("color", "white"),
						)
				case LegendTypeTag          =>
					graph = legend
					Seq(
						DotAttr("color", "black"),
						DotAttr("width", 1))
				case ChipTypeTag            =>
					Seq()
				case InvisibleSpacerTypeTag =>
					Seq(
						DotAttr("shape", "none"),
						DotAttr("fontcolor", "white"),
						)
				case _                      =>
					println(s"Unknown Graph Node Type Tag ${typeTag}")
					Seq()
			})

			Some((graph, DotNodeStmt(NodeId(ident), dattribs)))
		}
	}

	private case object LegendTypeTag extends NodeStyleTypeTag

	private case object ChipTypeTag extends NodeStyleTypeTag

	private case object InvisibleSpacerTypeTag extends NodeStyleTypeTag

	private case object TitleTypeTag extends NodeStyleTypeTag
}
