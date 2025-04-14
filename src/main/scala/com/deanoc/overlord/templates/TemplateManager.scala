package com.deanoc.overlord.templates

import com.deanoc.overlord.utils.Logging

import java.nio.file.{Files, Path, Paths}
import scala.sys.process._
import scala.jdk.CollectionConverters._
import scala.util.{Try, Success, Failure}

/** Manages templates for creating new projects. Supports local templates, git
  * repositories, and GitHub templates.
  */
object TemplateManager extends Logging {
  private val templateBasePath =
    Paths.get(System.getProperty("user.home"), ".overlord", "templates")

  /** Creates a new project from a template.
    *
    * @param templateName
    *   The name of the template to use
    * @param projectName
    *   The name of the project to create
    * @param outputPath
    *   The path where the project should be created
    * @return
    *   true if successful, false otherwise
    */
  def createFromTemplate(
      templateName: String,
      projectName: String,
      outputPath: String
  ): Boolean = {
    info(s"Creating project '$projectName' from template '$templateName'")

    // Ensure the template exists
    val templatePath = templateBasePath.resolve(templateName)
    if (!Files.exists(templatePath)) {
      error(s"Template '$templateName' not found")
      return false
    }

    // Create the output directory
    val projectPath = Paths.get(outputPath, projectName)
    if (Files.exists(projectPath)) {
      error(s"Project directory '$projectPath' already exists")
      return false
    }

    Files.createDirectories(projectPath)

    // Copy template files to the project directory
    copyDirectory(templatePath, projectPath)

    // Customize files with the project name
    customizeFiles(projectPath, projectName)

    info(s"Project '$projectName' created successfully at '$projectPath'")
    true
  }

  /** Lists all available templates.
    *
    * @return
    *   A list of template names
    */
  def listAvailableTemplates(): List[String] = {
    if (!Files.exists(templateBasePath)) {
      return List.empty
    }

    Try {
      Files
        .list(templateBasePath)
        .iterator()
        .asScala
        .filter(Files.isDirectory(_))
        .map(_.getFileName.toString)
        .toList
    }.getOrElse(List.empty)
  }

  /** Copies a directory recursively.
    *
    * @param source
    *   The source directory
    * @param target
    *   The target directory
    */
  private def copyDirectory(source: Path, target: Path): Unit = {
    Files.walk(source).iterator().asScala.foreach { path =>
      val relativePath = source.relativize(path)
      val targetPath = target.resolve(relativePath)

      if (Files.isDirectory(path)) {
        if (!Files.exists(targetPath)) {
          Files.createDirectory(targetPath)
        }
      } else {
        Files.copy(path, targetPath)
      }
    }
  }

  /** Customizes template files with the project name.
    *
    * @param projectPath
    *   The path to the project
    * @param projectName
    *   The name of the project
    */
  private def customizeFiles(projectPath: Path, projectName: String): Unit = {
    // Find files that need customization
    val filesToCustomize = Files
      .walk(projectPath)
      .iterator()
      .asScala
      .filter(Files.isRegularFile(_))
      .filter(p => {
        val fileName = p.getFileName.toString
        fileName.endsWith(".over") ||
        fileName.endsWith(".scala") ||
        fileName.endsWith(".md") ||
        fileName == "build.sbt"
      })
      .toList

    // Replace template placeholders with the project name
    filesToCustomize.foreach { file =>
      val content = new String(Files.readAllBytes(file), "UTF-8")
      val customized = content
        .replace("{{project_name}}", projectName)
        .replace("{{PROJECT_NAME}}", projectName.toUpperCase)
        .replace("{{project-name}}", projectName.toLowerCase.replace(" ", "-"))

      Files.write(file, customized.getBytes("UTF-8"))
    }
  }
}
