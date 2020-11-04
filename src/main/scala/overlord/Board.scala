package overlord

import java.nio.file.{Files, Paths}

import toml._

import scala.collection.{immutable, mutable}

sealed trait BoardType {
	val defaultConstraints: Map[String, toml.Value]
}

case class XilinxBoard() extends BoardType {
	override val defaultConstraints: Map[String, Value] =
		immutable.Map[String, toml.Value](
			("pullup" -> toml.Value.Bool(false)),
			("slew" -> toml.Value.Str("Slow")),
			("drive" -> toml.Value.Num(8)),
			("direction" -> toml.Value.Str("None")),
			("standard" -> toml.Value.Str("LVCMOS33"))
			)
}

case class AlteraBoard() extends BoardType {
	override val defaultConstraints: Map[String, Value] =
		immutable.Map[String, toml.Value]()
}

case class LatticeBoard() extends BoardType {
	override val defaultConstraints: Map[String, Value] =
		immutable.Map[String, toml.Value]()
}

case class Board(boardType: BoardType,
                 name: String,
                 instances: List[toml.Value],
                 constraints: Map[String, Constraint])

object Board {
	def FromFile(spath: String,
	             name: String,
	             catalogs: DefinitionCatalogs): Option[Board] = {
		println(s"Reading $name board")

		val path = Paths.get(s"$spath/${name}.toml")
		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${name} board at ${path} not found");
			return None
		}

		val chipFile = path.toAbsolutePath.toFile
		val source   = scala.io.Source.fromFile(chipFile)

		parse(name, source.getLines mkString "\n", catalogs)
	}

	private def parse(name: String,
	                  source: String,
	                  catalogs: DefinitionCatalogs): Option[Board] = {
		val parsed    = {
			val tparsed = toml.Toml.parse(source)
			if (tparsed.isLeft) {
				println(
					s"$name board has failed to parse with error ${tparsed.left.get}"
					);
				return None
			}
			tparsed.right.get.values
		}
		// what type of board?
		val boardType = parsed("type").asInstanceOf[Value.Str].value match {
			case "Xilinx"  => XilinxBoard()
			case "Altera"  => AlteraBoard()
			case "Lattice" => LatticeBoard()
			case _         => println(s"$name board has a unknown type");
				return None
		}

		// parse defaults
		var defaultConstraints =
			mutable.HashMap[String, Value](boardType.defaultConstraints.toSeq: _*)

		if (parsed.contains("constraints_default")) {
			val defaultsTable =
				parsed("constraints_default").asInstanceOf[Value.Tbl].values
			for {
				(defaultName, defaultValue) <- defaultConstraints
				if (defaultsTable.contains(defaultName))
			} defaultConstraints(defaultName) = defaultsTable(defaultName)
		}
		if (!parsed.contains("type")) {
			println(s"${name} board requires a type value");
			return None
		}
		var boardChips = mutable.HashMap[String, Instance[_]]()

		// search catalogs for chips on the board
		val boardInstances   = if (parsed.contains("instance")) {
			parsed("instance").asInstanceOf[Value.Arr].values
		} else List[toml.Value]()
		var boardConstraints = mutable.HashMap[String, Constraint]()

		if (parsed.contains("constraint")) {
			val tconstraints =
				parsed("constraint").asInstanceOf[Value.Arr].values
			for (constraint <- tconstraints) {
				val table = constraint.asInstanceOf[Value.Tbl].values
				boardConstraints ++= Constraint.parse(table, defaultConstraints.toMap)
			}
		}

		Some(
			Board(
				boardType,
				name,
				boardInstances,
				boardConstraints.toMap
				)
			)
	}
}
