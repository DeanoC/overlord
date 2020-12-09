package overlord

import java.nio.file.{Files, Path}
import overlord.Definitions.{DefinitionTrait, DefinitionType}
import overlord.Instances.BoardInstance

import scala.collection.mutable
import scala.language.postfixOps

case class Resources(path: Path) {
	def loadCatalogs(): Map[DefinitionType, DefinitionTrait] = {
		import toml.Value
		val parsed =
			loadToToml(path.resolve("catalogs.toml").toAbsolutePath).values

		if (!parsed.contains("resources")) return Map()
		if (!parsed("resources").isInstanceOf[Value.Arr]) return Map()

		val resources = parsed("resources").asInstanceOf[Value.Arr].values

		val resourceMaps = mutable.HashMap[DefinitionType, DefinitionTrait]()

		(for (resource <- resources) yield {
			val name    = resource.asInstanceOf[toml.Value.Str].value
			DefinitionCatalog.fromFile(
					path.resolve("catalogs/"), s"$name")
		}).flatten.flatten.map(f => (f.defType -> f)).toMap
	}

	private def loadToToml(absolutePath: Path): toml.Value.Tbl = {
		if (!Files.exists(absolutePath)) {
			println(s"$absolutePath does't not exists");
			return toml.Value.Tbl(Map[String, toml.Value]())
		}

		val file       = absolutePath.toFile
		val sourcetext = io.Source.fromFile(file)
		val source     = sourcetext.getLines().mkString("\n")

		val tparsed = toml.Toml.parse(source)
		if (tparsed.isLeft) {
			println(
				s"${absolutePath} has failed to parse with error ${tparsed.left}"
				);
			toml.Value.Tbl(Map[String, toml.Value]())
		} else tparsed.toOption.get
	}
}
