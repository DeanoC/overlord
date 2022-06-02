package overlord

import ikuy_utils.{ArrayV, Utils, Variant}
import overlord.Instances.BoardInstance

import java.nio.file.Path
import scala.collection.mutable
import scala.language.postfixOps

object Resources {
	def stdResourcePath(): Path = {
		overlordRootPath().resolve("src/main/resources/")
	}

	def overlordRootPath(): Path =
		Path.of(new java.io.File(classOf[BoardInstance]
			                         .getProtectionDomain
			                         .getCodeSource
			                         .getLocation.toURI).getCanonicalPath)
			.getParent.getParent.getParent
}

case class Resources(path: Path) {
	def loadCatalogs(): Map[DefinitionType, DefinitionTrait] = {
		val parsed = Utils.readToml(path.resolve("catalogs.toml"))

		if (!parsed.contains("resources")) {
			println("no resources array in catalog.toml")
			return Map()
		}

		if (!parsed("resources").isInstanceOf[ArrayV]) {
			println("resources in catalog.toml isn't an array")
			return Map()
		}

		Game.pushCatalogPath("catalogs/")

		val resources = Utils.toArray(parsed("resources"))

		val result = (for (resource <- resources) yield {
			val name = Utils.toString(resource)
			DefinitionCatalog.fromFile(s"$name", Map[String, Variant]())
		}).flatten.flatten.map(f => f.defType -> f).toMap

		Game.popCatalogPath()
		result
	}

	def loadPrefabs(): Map[String, Prefab] = {
		val parsed =
			Utils.readToml(path.resolve("prefabs.toml"))

		if (!parsed.contains("resources")) {
			println("no resources array in prefabs.toml")
			return Map()
		}

		if (!parsed("resources").isInstanceOf[ArrayV]) {
			println("resources in prefabs.toml isn't an array")
			return Map()
		}

		Game.pushInstancePath("prefabs/")

		val resources = Utils.toArray(parsed("resources"))
		val prefabs   = mutable.Map[String, Prefab]()
		for (resource <- resources) {
			val name = Utils.toString(resource)
			Game.pushInstancePath(name)
			prefabs ++= PrefabCatalog.fromFile(s"$name").map(f => {
				(f.name.replace(".toml", "") -> f)
			})
			Game.popInstancePath()
		}

		Game.popInstancePath()

		prefabs.toMap
	}
}
