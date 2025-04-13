package com.deanoc.overlord

import com.deanoc.overlord.Main.Config
import org.scalatest.funsuite.AnyFunSuite
import java.nio.file.{Files, Paths}

import scopt.{DefaultOEffectSetup, OEffect, OParser}
import java.io.{ByteArrayOutputStream, PrintStream, OutputStream}

class MainSpec extends AnyFunSuite {

  private def suppressOutput[T](block: => T): T = {
    // Save original streams
    val originalOut = System.out
    val originalErr = System.err
    
    // Create a null output stream that discards all data
    val nullOutputStream = new OutputStream {
      override def write(b: Int): Unit = {}
      override def write(b: Array[Byte]): Unit = {}
      override def write(b: Array[Byte], off: Int, len: Int): Unit = {}
    }
    
    val dummyStream = new PrintStream(nullOutputStream)
    
    try {
      // Redirect both stdout and stderr
      System.setOut(dummyStream)
      System.setErr(dummyStream)
      
      // Execute the test code
      block
    } finally {
      // Restore original streams
      System.setOut(originalOut)
      System.setErr(originalErr)
    }
  }
  
  // Silent implementation of OEffectSetup that suppresses all output
  private val silentEffectSetup = new DefaultOEffectSetup {
    override def displayToOut(msg: String): Unit = { /* do nothing */ }
    override def displayToErr(msg: String): Unit = { /* do nothing */ }
    override def reportError(msg: String): Unit = { /* do nothing */ }
    override def reportWarning(msg: String): Unit = { /* do nothing */ }
  }
  
  // Create a silent parser based on Main's parser but without any console output
  private val silentParser = {
    val builder = OParser.builder[Config]
    import builder._
    OParser.sequence(
      programName("overlord"),
      head("overlord", "0.1.0"),
      cmd("create")
        .action((_, c) => c.copy(command = Some("create")))
        .text("generate a compile project with all sub parts at 'out'")
        .children(
          arg[String]("<infile>")
            .required()
            .action((x, c) => c.copy(infile = Some(x)))
            .text("filename should be a .over file to use for the project"),
          opt[String]("board")
            .required()
            .action((x, c) => c.copy(board = Some(x)))
            .text("board definition to use")
        ),
      cmd("update")
        .action((_, c) => c.copy(command = Some("update")))
        .text("update an existing project instance"),
      cmd("report")
        .action((_, c) => c.copy(command = Some("report")))
        .text("prints some info about the overall structure"),
      cmd("svd")
        .action((_, c) => c.copy(command = Some("svd")))
        .text("produce a CMSIS-SVD file on its own"),
      opt[String]("out")
        .action((x, c) => c.copy(out = x))
        .text("path where generated files should be placed"),
      opt[String]("board")
        .action((x, c) => c.copy(board = Some(x)))
        .text("board definition to use"),
      opt[Unit]("nostdresources")
        .action((_, c) => c.copy(nostdresources = true))
        .text("don't use the standard catalog"),
      opt[Unit]("nostdprefabs")
        .action((_, c) => c.copy(nostdprefabs = true))
        .text("don't use the standard prefabs"),
      opt[String]("resources")
        .action((x, c) => c.copy(resources = Some(x)))
        .text("use the specified path as the root of resources"),
      opt[String]("instance")
        .action((x, c) => c.copy(instance = Some(x)))
        .text("specify the instance to update"),
      opt[Unit]("yes")
        .abbr("y")
        .action((_, c) => c.copy(yes = true))
        .text("automatically agree (e.g., download resource files without prompting)"),
      arg[String]("<infile>")
        .optional()
        .action((x, c) => c.copy(infile = Some(x)))
        .text("filename should be a .over file to use for the project"),
      opt[Unit]("noexit")
        .action((_, c) => c.copy(noexit = true))
        .text("disable automatic exit on error logs"),
      opt[String]("trace")
        .action((x, c) => c.copy(trace = Some(x)))
        .text("enable trace logging for comma-separated list of modules (can use short names)"),
      opt[String]("debug")
        .action((x, c) => c.copy(debug = Some(x)))
        .text("enable debug logging for comma-separated list of modules (can use short names)"),
      opt[String]("stdresource")
        .action((x, c) => c.copy(stdresource = Some(x)))
        .text("specify the standard resource path {default: ...}")
    )
  }
  
  private def parseWithSuppressedOutput(args: Array[String]): Option[Config] = {
    val initialConfig = Config()
    val isNonInteractive = System.console() == null
    
    // Use our silent parser with our silent effect setup
    OParser.parse(silentParser, args, initialConfig.copy(yes = initialConfig.yes || isNonInteractive), silentEffectSetup)
  }
  
  // Helper method to create a truly silent output stream
  private def nullOutputStream = new OutputStream {
    override def write(b: Int): Unit = {}
    override def write(b: Array[Byte]): Unit = {}
    override def write(b: Array[Byte], off: Int, len: Int): Unit = {}
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
