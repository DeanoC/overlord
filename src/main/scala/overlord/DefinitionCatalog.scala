package overlord

import java.nio.file.{Files, Path}

import scala.collection.mutable.ArrayBuffer

case class DefinitionCatalog(val catalogName: String,
                             val defs: Seq[Definition])

case class DefinitionCatalogs(val catalogs: Map[String, DefinitionCatalog]) {
	def FindDefinition(defType: String): Option[Definition] = {
		val chipPath                 = defType.split('.')
		var bestMatch                = 0
		var defi: Option[Definition] = None

		for {(_, cat) <- catalogs; d <- cat.defs} {
			val split = d.chipType.split('.')

			if (split.length >= chipPath.length) {
				var curMatch   = 0

				split.zipWithIndex.foreach(
					cti =>
						if ((cti._2 < chipPath.length) &&
						    (cti._2 == curMatch) &&
						    (cti._1 == chipPath(cti._2))) curMatch += 1)

				if (curMatch > bestMatch) {
					defi = Some(d)
					bestMatch = curMatch
				}
			}
		}
		if (defi.isEmpty)
			println(s"${defType} not found in any definition catalogs")

		defi
	}
}

object DefinitionCatalog {
	def fromFile(spath: Path, name: String): Option[DefinitionCatalog] = {
		println(s"Reading $name catalog")

		val path = spath.resolve(s"${name}.toml")

		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${name} catalog at ${path} not found");
			return None
		}

		val chipFile = path.toAbsolutePath.toFile
		val source   = scala.io.Source.fromFile(chipFile)

		parse(spath, name, source.getLines().mkString("\n"))
	}

	private def parse(spath: Path,
	                  name: String,
	                  src: String): Option[DefinitionCatalog] = {
		import toml.Value

		val parsed = {
			val parsed = toml.Toml.parse(src)
			parsed match {
				case Right(value) => value.values
				case Left(value)  => println(s"$name has failed to parse with " +
				                             s"error ${Left(parsed)}")
					return None
			}
		}

		val registerPath = if (parsed.contains("registerPath")) {
			spath.resolve(parsed("registerPath").asInstanceOf[Value.Str].value)
		} else spath

		var defs = ArrayBuffer[Definition]()

		if (parsed.contains("definition")) {
			val chips = parsed("definition").asInstanceOf[Value.Arr].values

			for (chip <- chips) {
				val table   = chip.asInstanceOf[Value.Tbl].values
				val defType = table("type").asInstanceOf[Value.Str].value

				val software: Option[Software] =
					if (table.contains("software")) {
						val arr  = table("software").asInstanceOf[Value.Arr].values
						val regs = for (tname <- arr) yield {
							val name = tname.asInstanceOf[Value.Str].value
							Registers.fromResource(
								registerPath.resolve(Path.of(s"$name" + s".toml")))
						}
						Some(Software(regs.flatten.toArray))
					} else None

				val attribs = table.filter(a => a._1 match {
					case "type" | "software" => false
					case _                   => true
				})

				defs += Definition(defType, attribs, software)
			}
		}

		Some(DefinitionCatalog(name, defs.toSeq))
	}
}