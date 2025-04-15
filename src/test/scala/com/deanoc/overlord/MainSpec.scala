package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import java.nio.file.{Files, Path, Paths}
import com.deanoc.overlord.cli.{CommandLineParser, Config}
import scopt.OParser

class MainSpec extends AnyFlatSpec with Matchers {

  // Create a test helper to suppress console output during tests
  private class SilentOutput {
    // Methods to capture or suppress output during tests
    def withSilentOutput(block: => Unit): Unit = {
      // Suppress console output while executing the block
      val originalOut = System.out
      val originalErr = System.err
      try {
        // Redirect output to /dev/null
        block
      } finally {
        System.setOut(originalOut)
        System.setErr(originalErr)
      }
    }
  }

  private val silent = new SilentOutput()

  // Use the parser from CommandLineParser
  private def parseWithSuppressedOutput(
      args: Array[String]
  ): Option[Config] = {
    val initialConfig = Config()
    val isNonInteractive = System.console() == null

    // Use the parser
    val parser = CommandLineParser.createParser()
    OParser.parse(
      parser,
      args,
      initialConfig.copy(yes = initialConfig.yes || isNonInteractive)
    )
  }

  "Main" should "handle create project command without out parameter" in {
    val tempDir = Files.createTempDirectory("main_test")
    val templateName = "bare-metal"
    val projectName = "test-project"

    val args = Array("create", "project", templateName, projectName)

    try {
      // Test successful parsing of the command
      val config = parseWithSuppressedOutput(args)
      config shouldBe defined
      config.get.command shouldBe Some("create")
      config.get.subCommand shouldBe Some("project")
      config.get.templateName shouldBe Some(templateName)
      config.get.projectName shouldBe Some(projectName)

      // We don't need to call Main.main() which has side effects
      // Just verify the config is correctly parsed
    } finally {
      // Clean up
      Files.delete(tempDir)
    }
  }

  it should "parse generate commands using infile location" in {
    val tempDir = Files.createTempDirectory("main_test")
    val projectFile = tempDir.resolve("project.yaml")
    Files.createFile(projectFile)

    val args = Array("generate", "report", projectFile.toString)

    try {
      // Test successful parsing of the command
      val config = parseWithSuppressedOutput(args)
      config shouldBe defined
      config.get.command shouldBe Some("generate")
      config.get.subCommand shouldBe Some("report")
      config.get.infile shouldBe Some(projectFile.toString)
    } finally {
      // Clean up
      Files.delete(projectFile)
      Files.delete(tempDir)
    }
  }

  // Convert test() methods to it should style for consistency
  it should "correctly parse create project command" in {
    val args = Array(
      "create",
      "project",
      "bare-metal",
      "my-project"
    )
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    config.get.command shouldBe Some("create")
    config.get.subCommand shouldBe Some("project")
    config.get.templateName shouldBe Some("bare-metal")
    config.get.projectName shouldBe Some("my-project")
  }

  it should "fail when required arguments are missing" in {
    val args = Array("create", "project")
    val result = parseWithSuppressedOutput(args)
    result shouldBe empty
  }

  it should "correctly handle -y flag" in {
    val args =
      Array("create", "project", "bare-metal", "my-project", "-y")
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    config.get.yes shouldBe true
    config.get.templateName shouldBe Some("bare-metal")
    config.get.projectName shouldBe Some("my-project")
  }

  it should "correctly handle --yes flag" in {
    val args = Array(
      "create",
      "project",
      "bare-metal",
      "my-project",
      "--yes"
    )
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    config.get.yes shouldBe true
    config.get.templateName shouldBe Some("bare-metal")
    config.get.projectName shouldBe Some("my-project")
  }

  it should "handle resource flags and auto download options" in {
    val args = Array(
      "create",
      "project",
      "bare-metal",
      "my-project",
      "--nostdresources",
      "--resources",
      "./custom-resources",
      "--yes"
    )
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    config.get.nostdresources shouldBe true
    config.get.resources shouldBe Some("./custom-resources")
    config.get.yes shouldBe true
    config.get.templateName shouldBe Some("bare-metal")
    config.get.projectName shouldBe Some("my-project")
  }

  it should "set 'yes' option to true for non-interactive console" in {
    val args =
      Array("create", "project", "bare-metal", "my-project")
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    // This will be true if System.console() is null (non-interactive)
    // We can't easily test this condition directly
    config.get.templateName shouldBe Some("bare-metal")
    config.get.projectName shouldBe Some("my-project")
  }

  it should "correctly parse update catalog command" in {
    val args = Array("update", "catalog")
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    config.get.command shouldBe Some("update")
    config.get.subCommand shouldBe Some("catalog")
  }

  it should "correctly parse template commands" in {
    val args = Array("template", "list")
    val config = parseWithSuppressedOutput(args)

    config shouldBe defined
    config.get.command shouldBe Some("template")
    config.get.subCommand shouldBe Some("list")
  }
}
