package overlord

import java.nio.file.{Files, Path}

import toml._

import scala.collection.{immutable, mutable}

sealed trait BoardType {
	val defaultConstraints: Map[String, toml.Value]
}

case class XilinxBoard(family: String, device: String) extends BoardType {
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
                 instances: Seq[Instance[_]],
                 constraints: Map[String, Constraint])

object Board {
	def FromFile(spath: Path,
	             name: String,
	             catalogs: DefinitionCatalogs): Option[Board] = {
		println(s"Reading $name board")

		GameBuilder.pathStack.push(spath)

		val path = spath.resolve(s"$name.toml")
		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${name} board at ${path} not found");
			GameBuilder.pathStack.pop()
			return None
		}

		val chipFile = path.toAbsolutePath.toFile
		val source   = scala.io.Source.fromFile(chipFile)

		val result = parse(name, source.getLines().mkString("\n"), catalogs)
		GameBuilder.pathStack.pop()
		result
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
			case "Xilinx"  =>
				if (!parsed.contains("family") || !parsed.contains("device")) {
					println(s"$name Xilinx board requires a family AND device field")
					return None
				}
				XilinxBoard(parsed("family").asInstanceOf[Value.Str].value,
				            parsed("device").asInstanceOf[Value.Str].value)
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

		val boardConstraints: Map[String, Constraint] =
			if (parsed.contains("constraint")) {
				val tconstraints = parsed("constraint").asInstanceOf[Value.Arr].values
				(for (constraint <- tconstraints) yield {
					val table = constraint.asInstanceOf[Value.Tbl].values
					val t     = (table + ("type" -> Value.Str("port")))
					Constraint.parse(t, defaultConstraints.toMap)
				}).fold(Map[String, Constraint]())({ (m, c) => m ++ c }).toMap
			} else Map[String, Constraint]()

		// search catalogs for chips on the board
		val boardInstances: Seq[Instance[_]] = boardConstraints.map(bc => {
			val attribs = bc._2.attributes
			ConstraintInstance(bc._1,
			                   Def("port", None, attribs, Seq[Software]()),
			                   bc._2,
			                   attribs)
		}).toSeq.++(if (parsed.contains("instance")) {
			val instances = parsed("instance").asInstanceOf[Value.Arr].values
			for (instance <- instances) yield
				GameBuilder.parseInstance(instance, catalogs)
		}.filter(_.nonEmpty).map(_.get) else Seq[Instance[_]]())

		Some(Board(boardType, name, boardInstances, boardConstraints))
	}

}
