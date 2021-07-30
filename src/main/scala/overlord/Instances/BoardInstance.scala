package overlord.Instances

import ikuy_utils._
import overlord.ChipDefinitionTrait
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
                         override val definition: ChipDefinitionTrait,
                         override val children: Seq[InstanceTrait]
                        ) extends ChipInstance with Container {

	override val physical: Boolean = false

	override def copyMutate[A <: ChipInstance](nid: String): BoardInstance =
		copy(ident = nid)

	override def copyMutateContainer(copy: MutContainer): Container = {
		BoardInstance(ident = ident,
		              boardType = boardType,
		              definition = definition,
		              children = copy.children.toSeq)
	}
}

object BoardInstance {

	def apply(name: String,
	          definition: ChipDefinitionTrait,
	          iattribs: Map[String, Variant]): Option[BoardInstance] = {

		val attribs = Utils.mergeAintoB(iattribs, definition.attributes)

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
		                   children = Seq() // we be fixed up in post
		                   ))
	}
}