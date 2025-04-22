package com.deanoc.overlord

import com.deanoc.overlord.config.{SourceConfig, SourceType, ConfigPaths}
import com.deanoc.overlord.utils.{Logging, Utils}

import java.nio.file.{Files, Path, Paths}
import sys.process._
import scala.util.boundary, boundary.break
import com.deanoc.overlord.config.CatalogFileConfig
import com.deanoc.overlord.config._
object SourceLoader extends Logging {

  /**
   * Loads content from a Git repository source.
   * Clones the repository if it doesn't exist, or uses the existing one.
   * Reads the content of a specified file within the repository.
   *
   * @param url The URL of the Git repository.
   * @param filePath The path to the file within the repository to load. Defaults to "catalog.yaml".
   * @return Either an error message or the raw content of the file.
   */
  private def loadFromGit[L: io.circe.Decoder, T](
      url: String,
      filePath: String = "catalog.yaml"
  ): Either[String, T] = {
    val catalogsDir = ConfigPaths.projectPath.resolve("catalogs")
    if (!Files.exists(catalogsDir)) {
      Files.createDirectories(catalogsDir)
    }

    val cloneFolderName = url.split('/').last.replaceAll(".git$", "")
    val repoPath = catalogsDir.resolve(cloneFolderName)

    if (!Files.exists(repoPath)) {
      info(s"Cloning repository from $url to $repoPath")
      val cloneCommand = s"git clone $url ${repoPath.toAbsolutePath}".!
      if (cloneCommand != 0) {
        Left(s"Failed to clone repository from $url")
      } else {
        val targetFile = repoPath.resolve(filePath)
        if (Files.exists(targetFile)) {
          loadFromFile[L, T](repoPath.resolve(filePath).toString)
        } else {
          Left(s"File '$filePath' not found in cloned repository at $repoPath")
        }
      }
    } else {
      info(s"Repository already exists at $repoPath, skipping clone.")
      // Optionally, add logic to pull latest changes here
      val targetFile = repoPath.resolve(filePath)
      if (Files.exists(targetFile)) {
        loadFromFile[L, T](repoPath.resolve(filePath).toString)
      } else {
        Left(s"File '$filePath' not found in existing repository at $repoPath")
      }
    }
  }

  /**
   * Loads content from a URL source.
   * TODO: Implement fetching from URL.
   *
   * @param url The URL to fetch content from.
   * @return Either an error message or the raw content from the URL.
   */
  private def loadFromUrl[T](
      url: String
  ): Either[String, T] = {
    // TODO: Implement fetching from URL (non-git)
    Left(s"Fetching from URL is not yet implemented: $url")
  }

  /**
   * Loads content from a local file source.
   *
   * @param filePath The path to the local file.
   * @return Either an error message or the raw content of the file.
   */
  private def loadFromFile[L: io.circe.Decoder, T](
        filePath: String
  ): Either[String, T] = {
    val absoluteFilePath = ConfigPaths.projectPath.resolve(filePath)
    Utils.loadFileToParse(absoluteFilePath) match {
      case Left(err) =>
        error(s"Failed to load file from path $absoluteFilePath: $err")
        Left(err)
      case Right(content) =>
        Utils.parseYaml[L](content) match {
          case Right(parsed) => Right(parsed.asInstanceOf[T])
          case Left(err) =>
            error(s"Failed to parse file from path $absoluteFilePath: $err")
            Left(err)
        }
    }
  }

  /**
   * Loads content based on the provided SourceConfig.
   *
   * @param sourceConfig The configuration for the source.
   * @return Either an error message or the loaded content. The content type
   *         depends on the SourceType: String for Git, Fetch, Local;
   *         Map[String, Any] for Inline.
   */
  def loadSource[L: io.circe.Decoder, T](sourceConfig: SourceConfig): Either[String, T] = {
  try {
    sourceConfig.`type` match {
      case SourceType.Git =>
        sourceConfig.url match {
          case Some(url) =>
            // use path as the filename if provided, otherwise default to "catalog.yaml"
            val filePathInRepo = sourceConfig.path.getOrElse("catalog.yaml")
            loadFromGit(url, filePathInRepo)
          case None => Left("Git source type requires a url.")
        }
      case SourceType.Fetch =>
        sourceConfig.url match {
          case Some(url) => loadFromUrl(url)
          case None => Left("Fetch source type requires a url.")
        }
      case SourceType.Local =>
        sourceConfig.path match {
          case Some(path) => 
            loadFromFile[L, T](path)
          case None => Left("Local source type requires a path.")
        }
      case SourceType.Inline =>
        sourceConfig.inline match {
          case Some(inlineJson) =>
            implicitly[io.circe.Decoder[L]].decodeJson(inlineJson) match {
              case Right(parsed) => Right(parsed.asInstanceOf[T])
              case Left(err)     => Left(s"Failed to decode inline content: ${err.getMessage}")
            }
          case None =>
            Left("Inline source type requires an inline JSON block.")
        }
      case null => Left(s"Unknown source type: ${sourceConfig.`type`}")
    }
  } catch {
    case ex: Exception =>
      error(s"An error occurred while processing ${sourceConfig}: ${ex.getMessage}")
      throw ex
  }
}
}