package com.deanoc.overlord

import com.deanoc.overlord.utils.{
  Logging,
  Utils,
  Variant
} // Import Utils and Variant
import com.deanoc.overlord.config.{
  CatalogFileConfig,
  DefinitionConfig,
  CatalogSourceConfig,
  ProjectFileConfig
} // Import catalog config case classes

import java.nio.file.{Files, Path, Paths}
import sys.process._
import scala.collection.mutable
import scala.collection.immutable.HashMap
import scala.util.boundary, boundary.break
import io.circe.yaml.parser
import io.circe.Decoder // Import Decoder explicitly
import io.circe.generic.auto._ // Automatic derivation for case classes (for nested structures)
import definitions.{Definition, DefinitionTrait}

object CatalogLoader extends Logging {

  def parseDefinitionCatalog(
      name: String,
      parsedConfig: CatalogFileConfig,
      defaultMap: Map[
        String,
        Any
      ] // Changed to Any as defaults can be various types
  ): Seq[DefinitionTrait] = boundary {

    // parse out any defaults updating the default table
    // TODO: For now, just use the defaultMap as is
    // We'll convert to Variant when needed
    val defaults = defaultMap

    // if we reference any catalogs, process them
    parsedConfig.catalogs.foreach { catSourceConfig =>
        processCatalogSource(catSourceConfig, defaults)
    }

    // process any definitions
    val defs = mutable.ArrayBuffer[DefinitionTrait]()
    parsedConfig.definitions.foreach { definitionConfig =>
      // Call the refactored Definition.apply which now takes DefinitionConfig
      // Convert defaults to Map[String, Variant]
      val defaultsAsVariant = defaults.map { case (k, v) =>
        k -> Utils.toVariant(v)
      }.toMap
      Definition(definitionConfig, defaultsAsVariant) match {
        case Right(defn: DefinitionTrait) => defs += defn
        case Left(errorMsg)               =>
          // Use the string version of error to avoid ambiguity
          this.error(
            s"Error creating definition ${definitionConfig.name}: $errorMsg",
            null
          )
      }
    }

    // final duplication check (should have been done during processing)
    // TODO: Update duplication check to work with DefinitionConfig
    // val identArray = defs.map(_.defType.ident.mkString(".")).toArray
    // for (i <- 0 until identArray.length)
    //   for (j <- i + 1 until identArray.length) {
    //     if (identArray(i) == identArray(j)) {
    //       warn(s"${identArray(i)} already exists in catalog")
    //     }
    //   }

    defs.toSeq
  }

  private def processCatalogSource(
      sourceConfig: CatalogSourceConfig,
      defaultMap: Map[String, Any]
  ): Seq[DefinitionTrait] = boundary {

    val sourceType = sourceConfig.`type`
    val contentPath = sourceType match {
      case "git" | "fetch" =>
        sourceConfig.url.getOrElse {
          error(s"Catalog source of type '$sourceType' missing 'url'")
          boundary.break(Seq.empty)
        }
      case "local" =>
        sourceConfig.path.getOrElse {
          error(s"Catalog source of type '$sourceType' missing 'path'")
          boundary.break(Seq.empty)
        }
      case _ =>
        error(s"Unknown catalog source type: $sourceType")
        boundary.break(Seq.empty)
    }

    sourceType match {
      case "git"   => loadFromGit(contentPath, defaultMap)
      case "fetch" => loadFromUrl(contentPath, defaultMap)
      case "local" => loadFromFile(contentPath, defaultMap)
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
  ): Seq[DefinitionTrait] = boundary {
    val filePath = Overlord.projectPath.resolve(
      fileName
    ) // Assuming local paths are relative to project root

    if (!Files.exists(filePath.toAbsolutePath)) {
      error(s"Local catalog file not found at $filePath")
      boundary.break(Seq.empty)
    }

    if (Files.size(filePath.toAbsolutePath) == 0) {
      info(s"Local catalog file is empty: $filePath")
      boundary.break(Seq.empty)
    }

    val parsedConfig: Either[io.circe.Error, CatalogFileConfig] = for {
      yamlString <- Right(scala.io.Source.fromFile(filePath.toFile).mkString)
      json <- parser.parse(yamlString)
      config <- json.as[CatalogFileConfig]
    } yield config

    parsedConfig match {
      case Left(err) =>
        error(s"Failed to parse catalog file $filePath: $err")
        boundary.break(Seq.empty)
      case Right(parsed) =>
        Overlord.pushCatalogPath(filePath.getParent)
        val result = parseDefinitionCatalog(
          filePath.toString,
          parsed,
          defaultMap
        ) // parsed is already CatalogFileConfig
        Overlord.popCatalogPath()
        result.toSeq
    }
  }

  def parsePrefabCatalog(
      name: String,
      parsedConfig: ProjectFileConfig, // Prefabs contain project-like structure
      defaultMap: Map[String, Any]
  ): Either[String, PrefabCatalog] = {

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
    val prefabPath = currentPath.resolve(s"$prefabName.yaml")

    if (Files.exists(prefabPath)) {
      return Some(prefabPath)
    }

    // Then, check in the standard prefab directories
    val prefabDirs = List(
      Overlord.projectPath.resolve("prefabs"),
      Overlord.projectPath.resolve("boards")
    )

    for (dir <- prefabDirs) {
      if (Files.exists(dir)) {
        val path = dir.resolve(s"$prefabName.yaml")
        if (Files.exists(path)) {
          return Some(path)
        }
      }
    }

    None
  }
}
