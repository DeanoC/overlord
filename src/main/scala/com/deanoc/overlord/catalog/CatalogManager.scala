package com.deanoc.overlord.catalog

import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.Resources

import java.nio.file.{Files, Path, Paths}
import scala.sys.process._
import scala.jdk.CollectionConverters._
import scala.util.{Try, Success, Failure}

/** Manages the catalog of hardware definitions. Provides functionality to
  * update the catalog from remote repositories.
  */
object CatalogManager extends Logging {

  /** Updates the catalog from the remote repository.
    *
    * @return
    *   true if successful, false otherwise
    */
  def updateCatalog(): Boolean = {
    info("Updating catalog from remote repository")

    val stdResourcePath = Resources.stdResourcePath()

    // Check if the catalog directory exists
    if (!Files.exists(stdResourcePath)) {
      info(
        s"Standard resource folder '$stdResourcePath' does not exist. Creating it..."
      )
      try {
        Files.createDirectories(stdResourcePath.getParent)
      } catch {
        case e: Exception =>
          error(s"Failed to create directory: ${e.getMessage}")
          return false
      }
    }

    // Check if it's a git repository
    val isGitRepo = Files.exists(stdResourcePath.resolve(".git"))

    if (isGitRepo) {
      // Update existing repository
      info("Updating existing catalog repository...")

      val pullCommand = Process(
        Seq("git", "pull"),
        new java.io.File(stdResourcePath.toString)
      )

      val pullResult = pullCommand.!
      if (pullResult != 0) {
        error("Error: Failed to update the catalog repository.")
        return false
      }

      // Update submodules
      info("Updating git submodules...")
      val updateCommand = Process(
        Seq("git", "submodule", "update", "--recursive", "--remote"),
        new java.io.File(stdResourcePath.toString)
      )

      val updateResult = updateCommand.!
      if (updateResult != 0) {
        warn("Warning: Failed to update git submodules.")
      } else {
        info("Git submodules successfully updated.")
      }
    } else {
      // Clone the repository
      info("Cloning catalog repository...")

      // Remove the directory if it exists but is not a git repository
      if (Files.exists(stdResourcePath)) {
        info("Removing existing non-git directory...")
        deleteDirectory(stdResourcePath)
      }

      val cloneCommand =
        s"git clone https://github.com/DeanoC/gagameos_stdcatalog.git $stdResourcePath"
      val cloneResult = cloneCommand.!

      if (cloneResult != 0) {
        error("Error: Failed to clone the catalog repository.")
        return false
      }

      // Initialize and update submodules
      info("Initializing git submodules...")
      val initCommand = Process(
        Seq("git", "submodule", "init"),
        new java.io.File(stdResourcePath.toString)
      )

      val initResult = initCommand.!
      if (initResult != 0) {
        warn("Warning: Failed to initialize git submodules.")
      } else {
        info("Updating git submodules...")
        val updateCommand = Process(
          Seq("git", "submodule", "update", "--recursive"),
          new java.io.File(stdResourcePath.toString)
        )

        val updateResult = updateCommand.!
        if (updateResult != 0) {
          warn("Warning: Failed to update git submodules.")
        } else {
          info("Git submodules successfully initialized and updated.")
        }
      }
    }

    info("Catalog successfully updated.")
    true
  }

  /** Deletes a directory recursively.
    *
    * @param path
    *   The path to delete
    */
  private def deleteDirectory(path: Path): Unit = {
    if (Files.exists(path)) {
      Files
        .walk(path)
        .sorted(java.util.Comparator.reverseOrder())
        .iterator()
        .asScala
        .foreach(Files.delete(_))
    }
  }
}
