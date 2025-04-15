package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import java.nio.file.{Files, Path, Paths}

class ProjectSpec extends AnyFlatSpec with Matchers {

  "Project" should "use project file's directory as output path" in {
    val tempDir = Files.createTempDirectory("project_test")
    val projectFile = tempDir.resolve("project.yaml")
    Files.createFile(projectFile)

    val catalogPath = tempDir.resolve("catalog")
    val prefabsPath = tempDir.resolve("prefabs")
    Files.createDirectory(catalogPath)
    Files.createDirectory(prefabsPath)

    Project.setupPaths(projectFile, catalogPath, prefabsPath)

    // Verify the output path is set to the project file's directory
    Project.outPath.toAbsolutePath shouldBe projectFile.getParent.toAbsolutePath

    // Clean up
    Files.delete(projectFile)
    Files.delete(catalogPath)
    Files.delete(prefabsPath)
    Files.delete(tempDir)
  }

  it should "use provided directory as output path when no file is specified" in {
    val tempDir = Files.createTempDirectory("project_test")
    val catalogPath = tempDir.resolve("catalog")
    val prefabsPath = tempDir.resolve("prefabs")
    Files.createDirectory(catalogPath)
    Files.createDirectory(prefabsPath)

    Project.setupPaths(tempDir, catalogPath, prefabsPath)

    // Verify the output path is set to the provided directory
    Project.outPath.toAbsolutePath shouldBe tempDir.toAbsolutePath

    // Clean up
    Files.delete(catalogPath)
    Files.delete(prefabsPath)
    Files.delete(tempDir)
  }

  it should "correctly resolve paths under the output directory" in {
    val tempDir = Files.createTempDirectory("project_test")
    val projectFile = tempDir.resolve("project.yaml")
    Files.createFile(projectFile)

    val catalogPath = tempDir.resolve("catalog")
    val prefabsPath = tempDir.resolve("prefabs")
    Files.createDirectory(catalogPath)
    Files.createDirectory(prefabsPath)

    Project.setupPaths(projectFile, catalogPath, prefabsPath)

    // Push a subdirectory onto the output path stack
    Project.pushOutPath("subdirectory")

    // Verify the output path includes the subdirectory
    Project.outPath.toAbsolutePath shouldBe
      projectFile.getParent.resolve("subdirectory").toAbsolutePath

    // Clean up
    Project.popOutPath()
    Files.delete(projectFile)
    Files.delete(catalogPath)
    Files.delete(prefabsPath)
    Files.delete(tempDir)
  }
}
