package overlord.Instances

import ikuy_utils._
import overlord.Connections.Connection
import overlord.{ChipDefinitionTrait, Definition}
import toml.Value

import java.nio.file.Path
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
                         override var children: Seq[InstanceTrait] = Seq()
                        ) extends ChipInstance with Container {

	override val physical   : Boolean         = true
	override var connections: Seq[Connection] = Seq()

	override def copyMutate[A <: ChipInstance](nid: String): BoardInstance =
		copy(ident = nid)
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

		if (!attribs.contains("clocks")) {
			println(s"${name} board requires some clocks");
			return None
		}

		if (!attribs.contains("pingroups")) {
			println(s"${name} board requires some pingroups");
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
		val defaults  = if (attribs.contains("defaults")) {
			Utils.toTable(attribs("defaults"))
		} else Map[String, Variant]()

		// instiatiate all pingroups
		val pingroups = (for (pinv <- Utils.toArray(attribs("pingroups"))) yield {
			val table    = Utils.toTable(pinv)
			val name     = Utils.toString(table("name"))
			val pingroup = table ++ Map[String, Variant]("type" -> StringV(s"pingroup.$name"))
			Definition(TableV(pingroup), Path.of("."), defaults).createInstance(s"$name",
			                                                                    pingroup)
		}).flatten.toSeq

		Some(BoardInstance(name,
		                   boardType = boardType,
		                   definition = definition,
		                   children = pingroups
		                   ))
	}
}