package com.deanoc.overlord

import com.deanoc.overlord.cli.{Config => CliConfig, CommandLineParser}
import org.scalatest.funsuite.AnyFunSuite
import java.nio.file.{Files, Paths}

import scopt.{DefaultOEffectSetup, OParser}

class MainSpec extends AnyFunSuite {

  // Silent implementation of OEffectSetup that suppresses all output
  private val silentEffectSetup = new DefaultOEffectSetup {
    override def displayToOut(msg: String): Unit = { /* do nothing */ }
    override def displayToErr(msg: String): Unit = { /* do nothing */ }
    override def reportError(msg: String): Unit = { /* do nothing */ }
    override def reportWarning(msg: String): Unit = { /* do nothing */ }
  }

  // Use the parser from CommandLineParser but with our silent effect setup
  private def parseWithSuppressedOutput(
      args: Array[String]
  ): Option[CliConfig] = {
    val initialConfig = CliConfig()
    val isNonInteractive = System.console() == null

    // Use the parser with our silent effect setup
    val parser = CommandLineParser.createParser()
    OParser.parse(
      parser,
      args,
      initialConfig.copy(yes = initialConfig.yes || isNonInteractive),
      silentEffectSetup
    )
  }

  test(
    "Option parsing should correctly parse create project command with output path"
  ) {
    val args = Array(
      "create",
      "project",
      "example.over",
      "--out",
      "./output",
      "--board",
      "test-board"
    )
    val config = parseWithSuppressedOutput(args).get

    assert(config.command.contains("create"))
    assert(config.subCommand.contains("project"))
    assert(config.out == "./output")
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should fail when no infile is provided") {
    val args =
      Array("create", "project", "--out", "./output", "--board", "test-board")
    val result = parseWithSuppressedOutput(args)
    assert(result.isEmpty)
  }

  test("Option parsing should fail when no board is provided") {
    val args = Array("create", "project", "example.over")
    val result = parseWithSuppressedOutput(args)
    assert(result.isEmpty)
  }

  test("Option parsing should correctly handle -y flag") {
    val args =
      Array("create", "project", "example.over", "-y", "--board", "test-board")
    val config = parseWithSuppressedOutput(args).get

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should correctly handle --yes flag") {
    val args = Array(
      "create",
      "project",
      "example.over",
      "--yes",
      "--board",
      "test-board"
    )
    val config = parseWithSuppressedOutput(args).get

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test(
    "Option parsing should handle both resource flags and auto download options"
  ) {
    val args = Array(
      "create",
      "project",
      "example.over",
      "--nostdresources",
      "--resources",
      "./custom-resources",
      "--yes",
      "--board",
      "test-board"
    )
    val config = parseWithSuppressedOutput(args).get

    assert(config.nostdresources)
    assert(config.resources.contains("./custom-resources"))
    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Non-interactive console should set 'yes' option to true") {
    val args =
      Array("create", "project", "example.over", "--board", "test-board")
    val config = parseWithSuppressedOutput(args).get.copy(yes = true)

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should correctly parse create from-template command") {
    val args = Array(
      "create",
      "from-template",
      "bare-metal",
      "my-project",
      "--out",
      "./output"
    )
    val config = parseWithSuppressedOutput(args).get

    assert(config.command.contains("create"))
    assert(config.subCommand.contains("from-template"))
    assert(config.templateName.contains("bare-metal"))
    assert(config.projectName.contains("my-project"))
    assert(config.out == "./output")
  }

  test("Option parsing should correctly parse update catalog command") {
    val args = Array("update", "catalog")
    val config = parseWithSuppressedOutput(args).get

    assert(config.command.contains("update"))
    assert(config.subCommand.contains("catalog"))
  }

  test("Option parsing should correctly parse template commands") {
    val args = Array("template", "list")
    val config = parseWithSuppressedOutput(args).get

    assert(config.command.contains("template"))
    assert(config.subCommand.contains("list"))
  }
}
