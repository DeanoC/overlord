package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import java.nio.file.{Files, Path, Paths}

class OverlordSpec extends AnyFlatSpec with Matchers {

  "Project" should "use project file's directory as output path" in {
    val tempDir = Files.createTempDirectory("project_test")
    val projectFile = tempDir.resolve("project.yaml")
    Files.createFile(projectFile)

    Overlord.setupPaths(projectFile)

    // Verify the output path is set to the project file's directory
    Overlord.outPath.toAbsolutePath shouldBe projectFile.getParent.toAbsolutePath

    // Clean up
    Files.delete(projectFile)
    Files.delete(tempDir)
  }

  it should "use provided directory as output path when no file is specified" in {
    val tempDir = Files.createTempDirectory("project_test")

    Overlord.setupPaths(tempDir)

    // Verify the output path is set to the provided directory
    Overlord.outPath.toAbsolutePath shouldBe tempDir.toAbsolutePath

    // Clean up
    Files.delete(tempDir)
  }

  it should "correctly resolve paths under the output directory" in {
    val tempDir = Files.createTempDirectory("project_test")
    val projectFile = tempDir.resolve("project.yaml")
    Files.createFile(projectFile)

    Overlord.setupPaths(projectFile)

    // Push a subdirectory onto the output path stack
    Overlord.pushOutPath("subdirectory")

    // Verify the output path includes the subdirectory
    Overlord.outPath.toAbsolutePath shouldBe
      projectFile.getParent.resolve("subdirectory").toAbsolutePath

    // Clean up
    Overlord.popOutPath()
    Files.delete(projectFile)
    Files.delete(tempDir)
  }
}
