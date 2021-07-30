package overlord

import ikuy_utils.{ArrayV, Utils, Variant}

import java.nio.file.{Files, Path}
import overlord.Instances.BoardInstance

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
}
