package overlord.Instances

import gagameos._
import overlord.Interfaces.UnconnectedLike
import overlord.{ChipDefinitionTrait, Definition}
import scala.util.boundary, boundary.break

import scala.collection.immutable

sealed trait BoardType {
	val defaults: Map[String, Variant]
}

case class XilinxBoard(family: String, device: String) extends BoardType {
	override val defaults: Map[String, Variant] =
		immutable.Map[String, Variant](
			("pullup" -> BooleanV(false)),
			("slew" -> StringV("Slow")),
			("drive" -> IntV(8)),
			("direction" -> StringV("None")),
			("standard" -> StringV("LVCMOS33"))
			)
}

case class AlteraBoard() extends BoardType {
	override val defaults: Map[String, Variant] =
		immutable.Map[String, Variant]()
}

case class LatticeBoard() extends BoardType {
	override val defaults: Map[String, Variant] =
		immutable.Map[String, Variant]()
}

case class GowinBoard(family: String, device: String) extends BoardType {
	override val defaults: Map[String, Variant] =
		immutable.Map[String, Variant](			
			("pullup" -> BooleanV(false)),
			("slew" -> StringV("Slow")),
			("drive" -> IntV(8)),
			("direction" -> StringV("None")),
			("standard" -> StringV("LVCMOS33"))
			)
}


case class BoardInstance(name: String,
                         boardType: BoardType,
                         override val definition: ChipDefinitionTrait,
                         override val children: Seq[InstanceTrait] = Seq()
                        ) extends ChipInstance with Container {
	override val physical   : Boolean              = true
	override val unconnected: Seq[UnconnectedLike] = Seq()

	override def isVisibleToSoftware: Boolean = true
}

object BoardInstance {

	def apply(name: String,
	          definition: ChipDefinitionTrait,
	          iattribs: Map[String, Variant]): Option[BoardInstance] = {

		val attribs = Utils.mergeAintoB(iattribs, definition.attributes)

		boundary { 
			if (!attribs.contains("board_type")) {
				println(s"${name} board requires a type value");
				break(None)
			}

			if (!attribs.contains("clocks")) {
				println(s"${name} board requires some clocks");
				break(None)
			}

			if (!attribs.contains("pingroups")) {
				println(s"${name} board requires some pingroups");
				break(None)
			}

			// what type of board?
			val boardType = Utils.toString(attribs("board_type")) match {
				case "Xilinx"  =>
					if (!attribs.contains("board_family") ||
					    !attribs.contains("board_device")) {
						println(s"$name Xilinx board requires a board_family AND " +
						        s"board_device field")
						break(None)
					}
					XilinxBoard(Utils.toString(attribs("board_family")),
					            Utils.toString(attribs("board_device")))
				case "Altera"  => AlteraBoard()
				case "Lattice" => LatticeBoard()
				case "Gowin"	=> 
					if (!attribs.contains("board_family") ||
					    !attribs.contains("board_device")) {
						println(s"$name Gowin board requires a board_family AND " +
						        s"board_device field")
						break(None)
					}
					GowinBoard(	Utils.toString(attribs("board_family")),
								Utils.toString(attribs("board_device")))
				case _         => println(s"$name board has a unknown board_type");
					break(None)
			}
			val defaults  = if (attribs.contains("defaults")) {
				Utils.toTable(attribs("defaults"))
			} else Map[String, Variant]()

			// instiatiate all clocks
			val clocks = (for (pinv <- Utils.toArray(attribs("clocks"))) yield {
				val table = Utils.toTable(pinv)
				if (table.contains("name")) {
					val name  = Utils.toString(table("name"))
					val clock = table ++ Map[String, Variant]("type" -> StringV(name))
					Definition(TableV(clock), defaults).createInstance(s"$name", clock)
				} else {
					println(s"clocks must either have a name field")
					break(None)
				}
			}).flatten.toSeq

			// instiatiate all pingroups
			val pingroups = (for (pinv <- Utils.toArray(attribs("pingroups"))) yield {
				val table = Utils.toTable(pinv)
				if (table.contains("name")) {
					val name     = Utils.toString(table("name"))
					val pingroup = table ++ Map[String, Variant]("type" -> StringV(s"pingroup.$name"))
					Definition(TableV(pingroup), defaults).createInstance(s"$name", pingroup)
				} else if (table.contains("names")) {
					val names = Utils.toArray(table("names"))
					(for (nameV <- names) yield {
						val name     = Utils.toString(nameV)
						val pingroup = table ++
						               Map[String, Variant]("type" -> StringV(s"pingroup.$name"))
						Definition(TableV(pingroup), defaults).createInstance(s"$name", pingroup)
					}).flatten.toSeq
				} else {
					println(s"pin groups must either have a name or names field")
					break(None)
				}
			}).flatten.toSeq

			Some(BoardInstance(name,
			                   boardType = boardType,
			                   definition = definition,
			                   children = clocks ++ pingroups
			                   ))
		}
	}
}