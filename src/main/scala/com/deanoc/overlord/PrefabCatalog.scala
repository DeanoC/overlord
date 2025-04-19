package com.deanoc.overlord

import com.deanoc.overlord.utils.{Logging, Utils}
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.config.{PrefabFileConfig, PrefabConfig}

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable
import scala.collection.immutable
import scala.util.boundary, boundary.break
import sys.process._
import io.circe.yaml.parser

// Represents a Prefab with its name, path, configuration, and included prefabs.
case class Prefab(
    name: String,
    path: String,
    config: PrefabFileConfig,
    prefabs: Map[String, Prefab] = Map.empty
)

// Documentation: How to Create a Prefab
// A Prefab represents a reusable component with a name, path, configuration, and included prefabs. To create a prefab:
// 1. Define a unique name for the prefab.
// 2. Specify the path where the prefab is located.
// 3. Provide a PrefabFileConfig containing instances, connections, resources, and prefabs.
// 4. Optionally, include other prefabs by providing a map of included prefab names to Prefab objects.
// 5. Use the PrefabCatalog class to manage and retrieve prefabs.

object PrefabCatalog {
  def apply(): PrefabCatalog = new PrefabCatalog()
}

class PrefabCatalog(
    val prefabs: PrefabCatalog#KeyStore = immutable.HashMap.empty
) extends Logging {
  type KeyStore = immutable.HashMap[String, Prefab]

  // Finds a prefab by its name.
  def findPrefab(name: String): Option[Prefab] = {
    // Check if the prefab exists in the catalog.
    if (prefabs.contains(name)) Some(prefabs(name))
    else None
  }
  
  // Loads a prefab from a file.
  def loadPrefabFromFile(path: Path): Either[String, Prefab] = boundary {
    if (!Files.exists(path)) {
      break(Left(s"Prefab file not found: $path"))
    }
    
    val prefabName = path.getFileName.toString.replaceAll("\\.yaml$", "")
    
    val parsedConfig: Either[io.circe.Error, PrefabFileConfig] = for {
      yamlString <- Right(scala.io.Source.fromFile(path.toFile).mkString)
      json <- parser.parse(yamlString)
      config <- json.as[PrefabFileConfig]
    } yield config
    
    parsedConfig match {
      case Left(err) =>
        break(Left(s"Failed to parse prefab file $path: $err"))
      case Right(config) =>
        Right(Prefab(prefabName, path.toString, config))
    }
  }
  
  // Adds a prefab to the catalog.
  def addPrefab(prefab: Prefab): PrefabCatalog = {
    new PrefabCatalog(prefabs + (prefab.name -> prefab))
  }
  
  // Loads all prefabs from a directory.
  def loadPrefabsFromDirectory(directory: Path): Either[String, PrefabCatalog] = boundary {
    if (!Files.exists(directory) || !Files.isDirectory(directory)) {
      break(Left(s"Prefab directory not found or not a directory: $directory"))
    }
    
    var catalog = this
    
    Files.list(directory)
      .filter(path => path.toString.endsWith(".yaml"))
      .forEach { path =>
        loadPrefabFromFile(path) match {
          case Left(err) =>
            warn(s"Failed to load prefab from $path: $err")
          case Right(prefab) =>
            catalog = catalog.addPrefab(prefab)
        }
      }
    
    Right(catalog)
  }
}