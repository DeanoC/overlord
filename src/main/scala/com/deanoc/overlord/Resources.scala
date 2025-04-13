package com.deanoc.overlord

import com.deanoc.overlord.utils.{ArrayV, Utils, Variant}
import com.deanoc.overlord.instances.BoardInstance
import com.deanoc.overlord.Project
import java.nio.file.{Path, Paths}
import scala.collection.mutable
import scala.language.postfixOps

object Resources {
  // Initialize with absolute path to avoid working directory issues
  private var stdResourcePathVar: Path = Paths
    .get(System.getProperty("user.home"), "gagameos_stdcatalog")
    .toAbsolutePath
    .normalize()

  def stdResourcePath(): Path = stdResourcePathVar

  def setStdResourcePath(path: Path): Unit = {
    stdResourcePathVar = path.toAbsolutePath.normalize()
  }

  // Helper method to resolve a path that might start with ~
  def resolvePath(pathString: String): Path = {
    val expanded = if (pathString.startsWith("~")) {
      pathString.replaceFirst("~", System.getProperty("user.home"))
    } else {
      pathString
    }
    Paths.get(expanded).toAbsolutePath.normalize()
  }

  def overlordRootPath(): Path =
    Paths
      .get(
        new java.io.File(
          classOf[
            BoardInstance
          ].getProtectionDomain.getCodeSource.getLocation.toURI
        ).getCanonicalPath
      )
      .getParent
      .getParent
      .getParent
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

    Project.pushCatalogPath("catalogs/")

    val resources = Utils.toArray(parsed("resources"))

    val result = (for (resource <- resources) yield {
      val name = Utils.toString(resource)
      DefinitionCatalog.fromFile(s"$name", Map[String, Variant]())
    }).flatten.flatten.map(f => f.defType -> f).toMap

    Project.popCatalogPath()
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

    Project.pushInstancePath("prefabs/")

    val resources = Utils.toArray(parsed("resources"))
    val prefabs = mutable.Map[String, Prefab]()
    for (resource <- resources) {
      val name = Utils.toString(resource)
      Project.pushInstancePath(name)
      prefabs ++= PrefabCatalog
        .fromFile(s"$name")
        .map(f => {
          (f.name.replace(".yaml", "") -> f)
        })
      Project.popInstancePath()
    }

    Project.popInstancePath()

    prefabs.toMap
  }
}
