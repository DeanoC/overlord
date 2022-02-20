package overlord

import ikuy_utils.{ArrayV, Utils, Variant}
import overlord.Instances.BoardInstance

import java.nio.file.Path
import scala.collection.mutable
import scala.language.postfixOps

object Resources {
	def projectRootPath(): Path =
		Path.of(new java.io.File(classOf[BoardInstance]
			                         .getProtectionDomain
			                         .getCodeSource
			                         .getLocation.toURI).getCanonicalPath)
			.getParent.getParent.getParent

	def stdResourcePath(): Path = {
		projectRootPath().resolve("src/main/resources/")
	}
}

case class Resources(path: Path) {
	def loadCatalogs(): Map[DefinitionType, DefinitionTrait] = {
		val parsed =
			Utils.readToml("catalogs.toml",
			               path.resolve("catalogs.toml"),
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
			                           path.resolve("catalogs/"),
			                           Map[String, Variant]())
		}).flatten.flatten.map(f => f.defType -> f).toMap
	}

	def loadPrefabs(): Map[String, Prefab] = {
		val parsed =
			Utils.readToml("prefabs.toml",
			               path.resolve("prefabs.toml"),
			               getClass)

		if (!parsed.contains("resources")) {
			println("no resources array in prefabs.toml")
			return Map()
		}

		if (!parsed("resources").isInstanceOf[ArrayV]) {
			println("resources in prefabs.toml isn't an array")
			return Map()
		}

		val resources = Utils.toArray(parsed("resources"))
		val prefabs   = mutable.Map[String, Prefab]()
		for (resource <- resources) {
			val name = Utils.toString(resource)
			prefabs ++= PrefabCatalog.fromFile(s"$name",
			                                   path.resolve("prefabs/")).map(f => (f.name ->
			                                                                       f))
		}

		prefabs.toMap
	}
}
