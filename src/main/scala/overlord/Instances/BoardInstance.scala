package overlord.Instances

import overlord.Connections.Connection
import overlord.Definitions.DefinitionTrait
import overlord.Gateware.{Parameter, Port}
import overlord.Utils
import toml.Value

import scala.collection.immutable

sealed trait BoardType {
	val defaults: Map[String, toml.Value]
}

case class XilinxBoard(family: String, device: String) extends BoardType {
	override val defaults: Map[String, Value] =
		immutable.Map[String, toml.Value](
			("pullup" -> toml.Value.Bool(false)),
			("slew" -> toml.Value.Str("Slow")),
			("drive" -> toml.Value.Num(8)),
			("direction" -> toml.Value.Str("None")),
			("standard" -> toml.Value.Str("LVCMOS33"))
			)
}

case class AlteraBoard() extends BoardType {
	override val defaults: Map[String, Value] =
		immutable.Map[String, toml.Value]()
}

case class LatticeBoard() extends BoardType {
	override val defaults: Map[String, Value] =
		immutable.Map[String, toml.Value]()
}

case class BoardInstance(ident: String,
                         boardType: BoardType,
                         definition: DefinitionTrait,
                         attributes: Map[String, Value],
                         override val children: Seq[Instance]
                        ) extends Instance with Container {

	override val physical:Boolean = false
	override def copyMutate[A <: Instance](nid: String,
	                                       nattribs: Map[String, Value])
	: BoardInstance =
		copy(ident = nid, attributes = nattribs)

	override def copyMutateContainer(copy: MutContainer): Container = {
		BoardInstance(ident = ident,
		              boardType = boardType,
		              definition = definition,
		              attributes = attributes,
		              children = copy.children.toSeq)
	}
}

object BoardInstance {
	def apply(name: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Value]): Option[BoardInstance] = {
		if (!attribs.contains("board_type")) {
			println(s"${name} board requires a type value");
			return None
		}

		// what type of board?
		val boardType = Utils.toString(attribs("board_type")) match {
			case "Xilinx"  =>
				if (!attribs.contains("board_family") ||
				    !attribs.contains("board_device")) {
					println(s"$name Xilinx board requires a board_family AND " +
					        s"board_device field")
					return None
				}
				XilinxBoard(Utils.toString(attribs("board_family")),
				            Utils.toString(attribs("board_device")))
			case "Altera"  => AlteraBoard()
			case "Lattice" => LatticeBoard()
			case _         => println(s"$name board has a unknown type");
				return None
		}

		Some(BoardInstance(name,
		                   boardType = boardType,
		                   definition = definition,
		                   attributes = attribs,
		                   children = Seq() // we be fixed up in post
		                   ))
	}
}