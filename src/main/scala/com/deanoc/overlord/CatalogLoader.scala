package com.deanoc.overlord

import com.deanoc.overlord.utils.{Logging, Utils, Variant} // Import Utils
import com.deanoc.overlord.config.{
  CatalogFileConfig,
  DefinitionConfig,
  SourceConfig,
  ComponentFileConfig,
  FileConfigBase
} // Added FileConfigBase to imports

import java.nio.file.{Files, Path, Paths}
import sys.process._
import scala.collection.mutable
import scala.collection.immutable.HashMap
import scala.util.boundary, boundary.break
import definitions.{Definition, DefinitionTrait}
import config.SourceType
import com.deanoc.overlord.config.ConfigPaths

object CatalogLoader extends Logging {
  // This is the main entry point for loading catalogs
  def processParsedCatalog(parsed: FileConfigBase): Seq[DefinitionTrait] = {

    info(s"Processing parsed catalogs ${parsed.catalogs.size}")
    parsed.catalogs.foreach { catSourceConfig =>
      processCatalogSource(catSourceConfig)
    }

    info(s"Processing parsed definitions ${parsed.definitions.size}")
    val defs = mutable.ArrayBuffer[DefinitionTrait]()
    parsed.definitions.foreach { definitionConfig =>
      Definition(definitionConfig) match {
        case Right(defn: DefinitionTrait) => 
          info(s"Loaded definition: ${defn.defType.toString()}")
          defs += defn
        case Left(err) =>
          error(s"Error creating definition ${definitionConfig.`type`}: $err")
      }
    }
    defs.toSeq
  }

  // This method processes the catalog source configuration
  private def processCatalogSource(
      sourceConfig: SourceConfig
  ): Seq[DefinitionTrait] = {
    info(s"Processing catalog from source: $sourceConfig")
    SourceLoader.loadSource[CatalogFileConfig, Seq[DefinitionTrait]](sourceConfig) match {
      case Right(content: Seq[DefinitionTrait]) => content
      case Left(err) =>
        error(s"Failed to load catalog from source ${SourceType.toString(sourceConfig.`type`)}: $err")
        Seq.empty
    }
  }
  
/*
  def parsePrefabCatalog(
      name: String,
      parsedConfig: ComponentFileConfig, // Can now also use parseDefinitionCatalog with this
      defaultMap: Map[String, Any]
  ): Either[String, PrefabCatalog] = {
    // Now we could potentially reuse parseDefinitionCatalog here with parsedConfig
    // since ComponentFileConfig also implements FileConfigBase

    Left("Prefab catalog parsing is being REMOVED.")
    
    var prefabCatalog = new PrefabCatalog()

    // Process prefabs from the project file
    parsedConfig.prefabs.foreach { prefabConfig =>
        val prefabName = prefabConfig.name
        val prefabPath = findPrefabPath(prefabName)

        if (prefabPath.isEmpty) {
          warn(s"Prefab not found: $prefabName")
        } else {
          val path = prefabPath.get

          // Load the prefab file
          prefabCatalog.loadPrefabFromFile(path) match {
            case Left(err) =>
              warn(s"Failed to load prefab $prefabName: $err")
            case Right(prefab) =>
              prefabCatalog = prefabCatalog.addPrefab(prefab)

              // Process included prefabs recursively
              prefab.config.include.foreach { includes =>
                for (include <- includes) {
                  val includedName = include.resource
                  val includedPath = findPrefabPath(includedName)

                  if (includedPath.isEmpty) {
                    warn(s"Included prefab not found: $includedName")
                  } else {
                    prefabCatalog.loadPrefabFromFile(includedPath.get) match {
                      case Left(err) =>
                        warn(
                          s"Failed to load included prefab $includedName: $err"
                        )
                      case Right(includedPrefab) =>
                        prefabCatalog = prefabCatalog.addPrefab(includedPrefab)
                    }
                  }
                }
              }
          }
        }
    }

    Right(prefabCatalog)
  }

  // Helper method to find the path of a prefab by its name
  private def findPrefabPath(prefabName: String): Option[Path] = {
    // First, check in the current catalog path
    val currentPath = Overlord.catalogPath
    val prefabFile = s"$prefabName.yaml"
    val prefabPath = currentPath.resolve(prefabFile)

    if (Files.exists(prefabPath)) {
      Some(prefabPath)
    } else {
      // Then, check in the standard prefab directories
      val prefabDirs = List(
        Overlord.projectPath.resolve("prefabs"),
        Overlord.projectPath.resolve("boards")
      )

      prefabDirs.find(dir => Files.exists(dir) && Files.exists(dir.resolve(prefabFile)))
        .map(_.resolve(prefabFile))
    }
  }
*/
}
