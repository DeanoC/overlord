package overlord

import java.nio.file.{Files, Path}
import overlord.Definitions.{DefinitionTrait, DefinitionType}

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

	def loadBoards(catalogs: DefinitionCatalog): Map[String, Board] = {

		GameBuilder.pathStack.push(path)

		val parsed = loadToToml(path.resolve("boards.toml").toAbsolutePath)
		val result = if (
			!parsed.values.contains("resources") || !parsed
				.values("resources")
				.isInstanceOf[toml.Value.Arr]
		) Map[String, Board]()
		else {
			val resources    =
				parsed.values("resources").asInstanceOf[toml.Value.Arr].values
			val resourceMaps = mutable.HashMap[String, Board]()
			for (resource <- resources) {
				val name  = resource.asInstanceOf[toml.Value.Str].value
				val board = Board.FromFile(path.resolve("boards"), name, catalogs)
				board match {
					case Some(b) => resourceMaps += (name -> b)
					case None    =>
				}
			}
			resourceMaps.toMap
		}
		GameBuilder.pathStack.pop()
		result
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
