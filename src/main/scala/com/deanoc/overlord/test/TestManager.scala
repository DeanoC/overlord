package com.deanoc.overlord.test

import com.deanoc.overlord.utils.Logging

import java.nio.file.{Files, Path, Paths}
import scala.sys.process._
import scala.jdk.CollectionConverters._
import scala.util.{Try, Success, Failure}
import scala.io.Source

/** Manages test generation and cleanup for Overlord projects.
  */
object TestManager extends Logging {

  /** Generates test files for a project.
    *
    * @param projectName
    *   The name of the project
    * @return
    *   true if successful, false otherwise
    */
  def generateTests(projectName: String): Boolean = {
    info(s"Generating tests for project '$projectName'")

    // Find the project directory
    findProjectPath(projectName) match {
      case Some(projectPath) =>
        // Create test directory if it doesn't exist
        val testPath =
          projectPath.resolve("src").resolve("test").resolve("scala")
        if (!Files.exists(testPath)) {
          Files.createDirectories(testPath)
        }

        // Find source files to create tests for
        val sourceFiles = findSourceFiles(projectPath)

        if (sourceFiles.isEmpty) {
          warn(s"No source files found in project '$projectName'")
          return true
        }

        // Generate test files
        var generatedCount = 0
        sourceFiles.foreach { sourceFile =>
          if (generateTestFile(sourceFile, testPath)) {
            generatedCount += 1
          }
        }

        info(s"Generated $generatedCount test files for project '$projectName'")
        true

      case None =>
        error(s"Project '$projectName' not found")
        false
    }
  }

  /** Cleans test files for a project.
    *
    * @param projectName
    *   The name of the project
    * @return
    *   true if successful, false otherwise
    */
  def cleanTests(projectName: String): Boolean = {
    info(s"Cleaning tests for project '$projectName'")

    // Find the project directory
    findProjectPath(projectName) match {
      case Some(projectPath) =>
        // Find and delete test files
        val testPath = projectPath.resolve("src").resolve("test")
        if (Files.exists(testPath)) {
          deleteDirectory(testPath)
          info(s"Removed test directory for project '$projectName'")
        } else {
          info(s"No test directory found for project '$projectName'")
        }

        true

      case None =>
        error(s"Project '$projectName' not found")
        false
    }
  }

  /** Finds the path to a project.
    *
    * @param projectName
    *   The name of the project
    * @return
    *   Some(path) if found, None otherwise
    */
  private def findProjectPath(projectName: String): Option[Path] = {
    // Look for the project in the current directory and common locations
    val possibleLocations = List(
      Paths.get(projectName),
      Paths.get(".", projectName),
      Paths.get("..", projectName)
    )

    possibleLocations.find(Files.exists(_))
  }

  /** Finds source files in a project.
    *
    * @param projectPath
    *   The path to the project
    * @return
    *   A list of source files
    */
  private def findSourceFiles(projectPath: Path): List[Path] = {
    val sourcePath = projectPath.resolve("src").resolve("main").resolve("scala")
    if (!Files.exists(sourcePath)) {
      return List.empty
    }

    Try {
      Files
        .walk(sourcePath)
        .iterator()
        .asScala
        .filter(Files.isRegularFile(_))
        .filter(_.toString.endsWith(".scala"))
        .toList
    }.getOrElse(List.empty)
  }

  /** Generates a test file for a source file.
    *
    * @param sourceFile
    *   The source file
    * @param testPath
    *   The path to the test directory
    * @return
    *   true if successful, false otherwise
    */
  private def generateTestFile(sourceFile: Path, testPath: Path): Boolean = {
    try {
      // Extract package and class name from source file
      val content = new String(Files.readAllBytes(sourceFile), "UTF-8")
      val packageName = extractPackageName(content)
      val className = sourceFile.getFileName.toString.replace(".scala", "")

      // Create package directories in test path
      val packagePath =
        packageName.split("\\.").foldLeft(testPath) { (path, pkg) =>
          val newPath = path.resolve(pkg)
          if (!Files.exists(newPath)) {
            Files.createDirectories(newPath)
          }
          newPath
        }

      // Create test file path
      val testFilePath = packagePath.resolve(s"${className}Spec.scala")

      // Skip if test file already exists
      if (Files.exists(testFilePath)) {
        info(s"Test file already exists for $className, skipping")
        return true
      }

      // Generate test file content
      val testContent = s"""package $packageName
                          |
                          |import org.scalatest.flatspec.AnyFlatSpec
                          |import org.scalatest.matchers.should.Matchers
                          |
                          |class ${className}Spec extends AnyFlatSpec with Matchers {
                          |  "$className" should "be tested" in {
                          |    // TODO: Implement test
                          |    true should be(true)
                          |  }
                          |}
                          |""".stripMargin

      // Write test file
      Files.write(testFilePath, testContent.getBytes("UTF-8"))
      info(s"Generated test file for $className")
      true
    } catch {
      case e: Exception =>
        error(
          s"Failed to generate test file for ${sourceFile.getFileName}: ${e.getMessage}"
        )
        false
    }
  }

  /** Extracts the package name from source code.
    *
    * @param content
    *   The source code
    * @return
    *   The package name
    */
  private def extractPackageName(content: String): String = {
    val packageRegex = """package\s+([^\s]+)""".r
    packageRegex.findFirstMatchIn(content) match {
      case Some(m) => m.group(1)
      case None    => "com.example"
    }
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
