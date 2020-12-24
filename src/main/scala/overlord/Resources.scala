package overlord

import ikuy_utils.{ArrayV, Utils}

import java.nio.file.{Files, Path}
import overlord.Definitions.{DefinitionTrait, DefinitionType}

import scala.language.postfixOps

case class Resources(path: Path) {
	def loadCatalogs(): Map[DefinitionType, DefinitionTrait] = {
		import toml.Value
		val parsed =
			Utils.readToml("catalogs.toml",
			               path.resolve("catalogs.toml").toAbsolutePath,
			               getClass)

		if (!parsed.contains("resources")) {
			println("no resources array in catalog.toml")
			return Map()
		}

		if (!parsed("resources").isInstanceOf[ArrayV]) {
			println("resources in catalog.toml isn't an array")
			return Map()
		}

		val resources = Utils.toArray(parsed("resources"))
		(for (resource <- resources) yield {
			val name = Utils.toString(resource)
			DefinitionCatalog.fromFile(s"$name",
			                           path.resolve("catalogs/"))
		}).flatten.flatten.map(f => (f.defType -> f)).toMap
	}
}
