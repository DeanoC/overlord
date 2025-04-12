import Main.Config
import Main.parser
import org.scalatest.funsuite.AnyFunSuite
import java.nio.file.{Files, Paths}

import scopt.OParser
import java.io.ByteArrayOutputStream
import java.io.PrintStream

class MainSpec extends AnyFunSuite {

  private def suppressOutput[T](block: => T): T = {
    val originalOut = System.out
    val originalErr = System.err
    val dummyStream = new PrintStream(new ByteArrayOutputStream())
    try {
      System.setOut(dummyStream)
      System.setErr(dummyStream)
      block
    } finally {
      System.setOut(originalOut)
      System.setErr(originalErr)
    }
  }

  test("Option parsing should correctly parse create command with output path") {
    val args = Array("create", "--out", "./output", "--board", "test-board", "example.over")
    val config = suppressOutput {
      OParser.parse(parser, args, Config()).get
    }

    assert(config.command.contains("create"))
    assert(config.out == "./output")
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should fail when no infile is provided") {
    val args = Array("create", "--out", "./output")
    suppressOutput {
      assert(OParser.parse(parser, args, Config()).isEmpty)
    }
  }

  test("Option parsing should fail when no board is provided") {
    val args = Array("create", "example.over")
    suppressOutput {
      assert(OParser.parse(parser, args, Config()).isEmpty)
    }
  }

  test("Option parsing should correctly handle -y flag") {
    val args = Array("create", "-y", "--board", "test-board", "example.over")
    val config = suppressOutput {
      OParser.parse(parser, args, Config()).get
    }

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should correctly handle --yes flag") {
    val args = Array("create", "--yes", "--board", "test-board", "example.over")
    val config = suppressOutput {
      OParser.parse(parser, args, Config()).get
    }

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Option parsing should handle both resource flags and auto download options") {
    val args = Array(
      "create", 
      "--nostdresources", 
      "--resources", "./custom-resources", 
      "--yes", 
      "--board", "test-board", 
      "example.over"
    )
    val config = suppressOutput {
      OParser.parse(parser, args, Config()).get
    }

    assert(config.nostdresources)
    assert(config.resources.contains("./custom-resources"))
    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }

  test("Non-interactive console should set 'yes' option to true") {
    val args = Array("create", "--board", "test-board", "example.over")
    val config = suppressOutput {
      OParser.parse(parser, args, Config()).get.copy(yes = true)
    }

    assert(config.yes)
    assert(config.board.contains("test-board"))
    assert(config.infile.contains("example.over"))
  }
}