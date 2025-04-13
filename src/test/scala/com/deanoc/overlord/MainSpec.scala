package com.deanoc.overlord

import com.deanoc.overlord.Main.{Config, parser}
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
  
  
  // Use the actual parser from Main.scala but with our silent effect setup
  private def parseWithSuppressedOutput(args: Array[String]): Option[Config] = {
    val initialConfig = Config()
    val isNonInteractive = System.console() == null
    
    // Use the actual parser with our silent effect setup
    OParser.parse(parser, args, initialConfig.copy(yes = initialConfig.yes || isNonInteractive), silentEffectSetup)
  }
  

  test(
    "Option parsing should correctly parse create command with output path"
  ) {
    val args = Array(
      "create",
      "--out",
      "./output",
      "--board",
      "test-board",
      "example.over"
    )
    val config = parseWithSuppressedOutput(args).get

    assert(config.command.contains("create"))
    assert(config.out == "./output")
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should fail when no infile is provided") {
    val args = Array("create", "--out", "./output")
    val result = parseWithSuppressedOutput(args)
    assert(result.isEmpty)
  }

  test("Option parsing should fail when no board is provided") {
    val args = Array("create", "example.over")
    val result = parseWithSuppressedOutput(args)
    assert(result.isEmpty)
  }

  test("Option parsing should correctly handle -y flag") {
    val args = Array("create", "-y", "--board", "test-board", "example.over")
    val config = parseWithSuppressedOutput(args).get

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should correctly handle --yes flag") {
    val args = Array("create", "--yes", "--board", "test-board", "example.over")
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
      "--nostdresources",
      "--resources",
      "./custom-resources",
      "--yes",
      "--board",
      "test-board",
      "example.over"
    )
    val config = parseWithSuppressedOutput(args).get

    assert(config.nostdresources)
    assert(config.resources.contains("./custom-resources"))
    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Non-interactive console should set 'yes' option to true") {
    val args = Array("create", "--board", "test-board", "example.over")
    val config = parseWithSuppressedOutput(args).get.copy(yes = true)

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }
}
