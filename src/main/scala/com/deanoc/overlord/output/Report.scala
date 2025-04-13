package com.deanoc.overlord.output

import scala.collection.mutable

import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.hardware.Registers
import com.deanoc.overlord.instances.InstanceTrait
import com.deanoc.overlord.instances.{
  BoardInstance,
  ChipInstance,
  Container,
  CpuInstance
}
import com.deanoc.overlord.Project

// Commented imports removed - they're not being used

object Report {
  // private type GraphType = Graph[(String, DefinitionType, NodeStyleTypeTag), DiEdge]
  // private type EdgeType = GraphType#EdgeT
  // private type EdgeResult = Option[(DotGraph, DotEdgeStmt)]
  // private type NodeType = GraphType#NodeT
  // private type NodeResult = Option[(DotGraph, DotNodeStmt)]
  // private val cpuDotAttribs      = Seq(DotAttr("shape", "diamond"),
  //                                      DotAttr("style", "filled"),
  //                                      DotAttr("fillcolor", "darkolivegreen"))
  // private val ioDotAttribs       = Seq(DotAttr("shape", "box"),
  //                                      DotAttr("style", "filled"),
  //                                      DotAttr("fillcolor", "lightblue"))
  // private val ramDotAttribs      = Seq(DotAttr("shape", "box"),
  //                                      DotAttr("style", "filled"),
  //                                      DotAttr("fillcolor", "tomato"))
  // private val storageDotAttribs  = Seq(DotAttr("shape", "box"),
  //                                      DotAttr("style", "filled"),
  //                                      DotAttr("fillcolor", "palevioletred"))
  // private val switchDotAttribs   = Seq(DotAttr("shape", "oval"),
  //                                      DotAttr("style", "filled"),
  //                                      DotAttr("fillcolor", "green"))
  // private val pinGroupDotAttribs = Seq(DotAttr("shape", "oval"),
  //                                      DotAttr("style", "filled"),
  //                                      DotAttr("fillcolor", "darkblue"))

  // private val defaultDotAttribs     = Seq(DotAttr("shape", "box"))
  // private val ramEdgeDotAttribs     = Seq(DotAttr("style", "dashed"))
  // private val switchEdgeDotAttribs  = Seq(DotAttr("style", "dashed"), DotAttr("color", "red"))
  // private val defaultEdgeDotAttribs = Seq(DotAttr("color", "black"))

  def apply(game: Project): Unit = {

    val sb = new mutable.StringBuilder
    val cpus = game.cpus

    val cpuTypes = cpus.map(_.definition).toSet
    sb ++= f"------------------%n"
    sb ++= f"${cpus.map(_.cpuCount).sum} CPU cores of ${cpuTypes.size} different types%n"
    sb ++= f"------------------%n"
    for (cput <- cpuTypes) {
      val chipType = cput.defType.ident.mkString
      val arch = Utils.lookupString(cput.attributes, "arch", "UNKNOWN")
      val bw = Utils.lookupInt(cput.attributes, "width", 32)

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

    val boardIndex = game.distanceMatrix.instanceArray
      .indexWhere(_.isInstanceOf[BoardInstance])
    sb ++= f"%n------------------%n"
    sb ++= f"Board Index = $boardIndex%n"
    sb ++= f"------------------%n"

    // solve for board connected pins
    val boardConnectedPins = mutable.HashMap[Int, Int]()
    for {
      connected <- game.connected
      if connected.isPinToChip
      (sp, ep) = game.distanceMatrix.indicesOf(connected)
      route = game.distanceMatrix.routeBetween(sp, ep)
      if route.contains(boardIndex)
    } boardConnectedPins += (sp -> ep)

    boardConnectedPins.foreach(b => {
      val first = game.distanceMatrix.instanceArray(b._1)
      val second = game.distanceMatrix.instanceArray(b._2)
      sb ++= f" ${first.name} ${second.name}%n"
    })

    sb ++= game.distanceMatrix.debugPrint

    sb ++= f"%n------------------%n"
    sb ++= f"Bus Distance Matrix%n"
    sb ++= f"------------------%n%n"

    sb ++= game.busDistanceMatrix.debugPrint

    Utils.writeFile(Project.outPath.resolve("report.txt"), sb.result())

//		outputDotGraph(game)
  }

  private def reportInstances(game: Project): String = {
    game.children.map(reportInstance(_)).mkString("")
  }

  private def reportContainer(
      container: Container,
      indentLevel: Int = 0
  ): String = {
    container.children.map(reportInstance(_, indentLevel)).mkString("")
  }

  private def reportInstance(
      instance: InstanceTrait,
      indentLevel: Int = 0
  ): String = {
    val sb = new mutable.StringBuilder

    val indent = "\t" * indentLevel
    sb ++= (indent + f"------------------%n")
    sb ++= (indent + instance.name + f"%n")
    val id = instance.definition.defType.ident.mkString(".")
    sb ++= (indent + f"type: $id%n")
    instance match {
      case ci: ChipInstance =>
        for (rb <- ci.banks) {
          val rl = Registers.registerListCache(rb.registerListName)
          sb ++= f"   ${rb.name} - ${rb.registerListName} ${rl.description}%n"
        }
      case _ =>
    }
    instance match {
      case c: Container =>
        sb ++= reportContainer(c, indentLevel + 1)
      case _ =>
    }

    sb.result()
  }

  // private sealed class NodeStyleTypeTag

  // case class GraphVizContent(graphs: Map[String, DotGraph]) {
  // 	def edgePrep(innerEdge: EdgeType): EdgeResult = {
  // 		val root = graphs("root")

  // 		def attributes(edef: DefinitionType,
  // 		               typeTag: NodeStyleTypeTag) = {
  // 			{
  // 				edef match {
  // 					case RamDefinitionType(ident)    => ramEdgeDotAttribs
  // 					case SwitchDefinitionType(ident) => switchEdgeDotAttribs
  // 					case _                           => defaultEdgeDotAttribs
  // 				}
  // 			} ++ {
  // 				typeTag match {
  // 					case TitleTypeTag | LegendTypeTag | InvisibleSpacerTypeTag =>
  // 						Seq(
  // 							DotAttr("style", "invis"),
  // 							DotAttr("minlen", 1)
  // 							)
  // 					case ChipTypeTag                                           =>
  // 						Seq()
  // 					case _                                                     =>
  // 						println(s"Unknown Graph Node Type Tag ${typeTag}")
  // 						Seq()
  // 				}
  // 			}
  // 		}

  // 		innerEdge.edge match {
  // 			case DiEdge(source, target) => {
  // 				val (sident, sdef, _)       = source.value
  // 				val (eident, edef, typeTag) = target.value
  // 				val dattribs                = attributes(edef, typeTag)
  // 				Some((root, DotEdgeStmt(sident, eident, dattribs)))
  // 			}
  // 		}
  // 	}

  // 	def nodePrep(innerNode: NodeType): NodeResult = {
  // 		val (ident, defType, typeTag) = innerNode.value

  // 		val root   = graphs("root")
  // 		val legend = graphs("legend")
  // 		val pins   = graphs("pins")

  // 		var graph: DotGraph = root

  // 		var dattribs = defType match {
  // 			case CpuDefinitionType(_)      => cpuDotAttribs
  // 			case IoDefinitionType(_)       => ioDotAttribs
  // 			case RamDefinitionType(_)      => ramDotAttribs
  // 			case StorageDefinitionType(_)  => storageDotAttribs
  // 			case SwitchDefinitionType(_)   => switchDotAttribs
  // 			case PinGroupDefinitionType(_) => graph = pins; pinGroupDotAttribs
  // 			case _                         => defaultDotAttribs
  // 		}
  // 		dattribs ++= (typeTag match {
  // 			case TitleTypeTag           =>
  // 				graph = legend
  // 				Seq(
  // 					DotAttr("color", "white"),
  // 					)
  // 			case LegendTypeTag          =>
  // 				graph = legend
  // 				Seq(
  // 					DotAttr("color", "black"),
  // 					DotAttr("width", 1))
  // 			case ChipTypeTag            =>
  // 				Seq()
  // 			case InvisibleSpacerTypeTag =>
  // 				Seq(
  // 					DotAttr("shape", "none"),
  // 					DotAttr("fontcolor", "white"),
  // 					)
  // 			case _                      =>
  // 				println(s"Unknown Graph Node Type Tag ${typeTag}")
  // 				Seq()
  // 		})

  // 		Some((graph, DotNodeStmt(NodeId(ident), dattribs)))
  // 	}
  // }

  // private case object LegendTypeTag extends NodeStyleTypeTag

  // private case object ChipTypeTag extends NodeStyleTypeTag

  // private case object InvisibleSpacerTypeTag extends NodeStyleTypeTag

  // private case object TitleTypeTag extends NodeStyleTypeTag
}
