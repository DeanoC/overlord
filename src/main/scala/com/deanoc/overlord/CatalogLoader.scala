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

object CatalogLoader extends Logging {
  // This is the main entry point for loading catalogs
  def processParsedCatalog(
      parsed: FileConfigBase,
      filePath: Path,
      currentDefaults: Map[String, Any]
  ): Seq[DefinitionTrait] = {

    val defaults = currentDefaults ++ parsed.defaults.map { case (k, v) =>
      k -> Utils.toVariant(v)
    }.toMap

    parsed.catalogs.foreach { catSourceConfig =>
      Overlord.pushCatalogPath(filePath.getParent)
      processCatalogSource(catSourceConfig, defaults)
      Overlord.popCatalogPath()
    }

    val defs = mutable.ArrayBuffer[DefinitionTrait]()
    parsed.definitions.foreach { definitionConfig =>
      val defaultsAsVariant = defaults.map { case (k, v) =>
        k -> Utils.toVariant(v)
      }.toMap
      Definition(definitionConfig, defaultsAsVariant) match {
        case Right(defn: DefinitionTrait) => defs += defn
        case Left(errorMsg) =>
          this.error(
            s"Error creating definition ${definitionConfig.name}: $errorMsg",
            null
          )
      }
    }
    defs.toSeq
  }

  // This method processes the catalog source configuration
  private def processCatalogSource(
      sourceConfig: SourceConfig,
      defaultMap: Map[String, Any]
  ): Seq[DefinitionTrait] = boundary {

    sourceConfig.`type` match {
      case SourceType.Git   => loadFromGit(sourceConfig.url.get, defaultMap)
      case SourceType.Fetch => loadFromUrl(sourceConfig.url.get, defaultMap)
      case SourceType.Local => loadFromFile(sourceConfig.path.get, defaultMap)
      case SourceType.Inline => sourceConfig.inline match {
        case Some(inlineMap) =>
          inlineMap.collect {
            case (_, value: DefinitionTrait) => value
          }.toSeq
        case None =>
          error("Inline source type requires an inline map.")
          Seq.empty
      }
      case _       => Seq.empty // Already handled above
    }
  }

  private def loadFromGit(
      url: String,
      defaultMap: Map[String, Any]
  ): Seq[DefinitionTrait] = boundary {
    val catalogsDir = Overlord.projectPath.resolve("catalogs")
    if (!Files.exists(catalogsDir)) {
      Files.createDirectories(catalogsDir)
    }

    val cloneFolderName = url.split('/').last.replaceAll(".git$", "")
    val repoPath = catalogsDir.resolve(cloneFolderName)

    if (!Files.exists(repoPath)) {
      info(s"Cloning repository from $url to $repoPath")
      val cloneCommand = s"git clone $url ${repoPath.toAbsolutePath}".!
      if (cloneCommand != 0) {
        error(s"Failed to clone repository from $url")
        boundary.break(Seq.empty)
      }
    } else {
      info(s"Repository already exists at $repoPath, skipping clone.")
      // Optionally, add logic to pull latest changes
    }

    val definitionCatalogFile = repoPath.resolve("catalog.yaml")
    if (Files.exists(definitionCatalogFile)) {
      loadFromFile(definitionCatalogFile.toString, defaultMap)
    } else {
      error(s"No catalog.yaml found in cloned repository at $repoPath")
      Seq.empty
    }
  }

  private def loadFromUrl(
      url: String,
      defaultMap: Map[String, Any]
  ): Seq[DefinitionTrait] = boundary {
    // TODO: Implement fetching from URL (non-git)
    error(s"Fetching from URL is not yet implemented: $url")
    boundary.break(Seq.empty)
  }

  private def loadFromFile(
      fileName: String,
      defaultMap: Map[String, Any]
  ): Seq[DefinitionTrait] = {
    val filePath = Overlord.projectPath.resolve(fileName)

    val yamlString = Utils.loadFileToParse(filePath)
    yamlString match {
      case Left(errorMsg) =>
        error(errorMsg)
        Seq.empty
      case Right(yaml) =>
        Utils.parseYaml[CatalogFileConfig](yaml) match {
          case Left(errorMsg) =>
            error(errorMsg)
            Seq.empty
          case Right(parsed) =>
            processParsedCatalog(parsed, filePath, defaultMap)
        }
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
