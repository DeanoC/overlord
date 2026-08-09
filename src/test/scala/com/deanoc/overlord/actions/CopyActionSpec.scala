package com.deanoc.overlord.actions

import com.deanoc.overlord.{DefinitionType, Overlord}
import com.deanoc.overlord.instances.ProgramInstance
import com.deanoc.overlord.software.SoftwareDefinition
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

import java.nio.file.Files

class CopyActionSpec extends AnyFlatSpec with Matchers {
  "CopyAction" should "copy source content from the software definition directory" in {
    val projectDir = Files.createTempDirectory("copy-action-project")
    val catalogDir = Files.createTempDirectory("copy-action-catalog")
    val projectFile = projectDir.resolve("project.yaml")
    Files.writeString(projectFile, "boards: []\n")
    val source = catalogDir.resolve("payload.txt")
    Files.writeString(source, "adapter source\n")

    try {
      Overlord.setupPaths(projectFile)
      val action = CopyAction("payload.txt", "txt", "payload.txt")
      val definition = SoftwareDefinition(
        DefinitionType("program.copy_fixture"),
        catalogDir,
        Map.empty,
        Map.empty,
        Seq.empty,
        catalogDir.resolve("copy_fixture.yaml"),
        ActionsFile(Seq(action))
      )
      val instance = ProgramInstance("copy_fixture", definition)

      action.execute(instance, Map.empty)

      Files.readString(projectDir.resolve("copy_fixture/payload.txt")) shouldBe
        "adapter source"
    } finally {
      deleteRecursively(projectDir)
      deleteRecursively(catalogDir)
      Overlord.resetPaths()
    }
  }

  private def deleteRecursively(path: java.nio.file.Path): Unit = {
    if (Files.isDirectory(path)) {
      val stream = Files.list(path)
      try stream.forEach(child => deleteRecursively(child))
      finally stream.close()
    }
    Files.deleteIfExists(path)
  }
}
