package overlord

import gagameos.{ArrayV, Utils, Variant}
import overlord.Instances.BoardInstance

import java.nio.file.{Path, Paths}
import scala.collection.mutable
import scala.language.postfixOps

object Resources {
	def stdResourcePath(): Path = {
		Paths.get("../gagameosstd_catalog/").toAbsolutePath.normalize()
	}

	def overlordRootPath(): Path =
		Paths.get(new java.io.File(classOf[BoardInstance]
			                         .getProtectionDomain
			                         .getCodeSource
			                         .getLocation.toURI).getCanonicalPath)
			.getParent.getParent.getParent
}

case class Resources(path: Path) {
	def loadCatalogs(): Map[DefinitionType, DefinitionTrait] = {
		val parsed = Utils.readYaml(path.resolve("catalogs.yaml"))

		if (!parsed.contains("resources")) {
			println("no resources array in catalogs.yaml")
			return Map()
		}

		if (!parsed("resources").isInstanceOf[ArrayV]) {
			println("resources in catalogs.yaml isn't an array")
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
			Utils.readYaml(path.resolve("prefabs.yaml"))

		if (!parsed.contains("resources")) {
			println("no resources array in prefabs.yaml")
			return Map()
		}

		if (!parsed("resources").isInstanceOf[ArrayV]) {
			println("resources in prefabs.yaml isn't an array")
			return Map()
		}

		Game.pushInstancePath("prefabs/")

		val resources = Utils.toArray(parsed("resources"))
		val prefabs   = mutable.Map[String, Prefab]()
		for (resource <- resources) {
			val name = Utils.toString(resource)
			Game.pushInstancePath(name)
			prefabs ++= PrefabCatalog.fromFile(s"$name").map(f => {
				(f.name.replace(".yaml", "") -> f)
			})
			Game.popInstancePath()
		}

		Game.popInstancePath()

		prefabs.toMap
	}
}
