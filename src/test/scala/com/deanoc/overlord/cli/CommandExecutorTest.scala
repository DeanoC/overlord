package com.deanoc.overlord.cli

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

/**
 * Test for CommandExecutor to ensure it correctly displays help text
 * for various command scenarios.
 */
class CommandExecutorTest extends AnyFlatSpec with Matchers {

  // Utility to capture stdout during test execution
  def withOutputCapture(test: => Unit): String = {
    CommandLineParser.clearPrintBuffer()
    test
    CommandLineParser.getPrintBuffer
  }

  // Helper to normalize whitespace for comparison
  def normalizeString(text: String): String = {
    text.split("\n")
      .map(_.trim.replaceAll("\\s+", " "))
      .filter(_.nonEmpty)
      .mkString("\n")
  }

  // Helper function to test help output for a command or subcommand
  def testHelpOutput(command: String, subcommand: Option[String], options: Map[String, String] = Map(), expectedReturnValue: Boolean = false): Unit = {
    val actual = withOutputCapture {
      val config = Config(
        command = Some(command),
        subCommand = subcommand,
        options = options
      )

      CommandLineParser.validateAndDisplayHelp(config, false) shouldBe expectedReturnValue
    }

    // Ensure the buffer is not empty
    actual should not be empty

    // Ensure the expected output is not empty
    val expected = subcommand match {
      case Some(sub) => HelpTextManager.getSubcommandHelp(command, sub)
      case None      => HelpTextManager.getCommandHelp(command)
    }
    expected should not be empty

    // Verify output matches what HelpTextManager would produce
    normalizeString(actual) shouldEqual normalizeString(expected)
  }

  // Test cases for various command scenarios
  it should "have the correct number of commands in the CLI parser" in {
    val expectedCommandCount = 6 // Updated to match the actual number of commands
    val actualCommandCount = CommandLineParser.getAllCommands.size

    actualCommandCount shouldEqual expectedCommandCount
  }

  it should "have the correct number of subcommands for the 'create' command" in {
    val expectedSubcommandCount = 3 // Updated to match the actual number of subcommands
    val actualSubcommandCount = CommandLineParser.getSubcommandsFor("create").size

    actualSubcommandCount shouldEqual expectedSubcommandCount
  }

  it should "display help for 'create' command with no subcommand" in {
    testHelpOutput("create", None)
  }

  it should "display help for create subcommand 'project' when required arguments are missing" in {
    testHelpOutput("create", Some("project"))
  }

  it should "display help for create subcommand 'gcc-toolchain' when required arguments are missing" in {
    testHelpOutput("create", Some("gcc-toolchain"))
  }

  it should "display error for create with an invalid subcommand" in {
    val actual = withOutputCapture {
      val config = Config(
        command = Some("create"),
        subCommand = Some("invalid"),
        options = Map()
      )

      CommandLineParser.validateAndDisplayHelp(config, false) shouldBe false
    }

    // Ensure the buffer is not empty
    actual should not be empty

    // Dynamically fetch the expected output from HelpTextManager
    val expected = HelpTextManager.getInvalidSubcommandHelp("create", "invalid")

    expected should not be empty

    // Verify the actual output matches the expected output
    normalizeString(actual) shouldEqual normalizeString(expected)
  }

  it should "display help for generate subcommand 'test' when required arguments are missing" in {
    testHelpOutput("generate", Some("test"))
  }

  it should "display help for generate subcommand 'report' when required arguments are missing" in {
    testHelpOutput("generate", Some("report"))
  }

  it should "display help for clean subcommand 'test' when required arguments are missing" in {
    testHelpOutput("clean", Some("test"))
  }

  it should "display help for update subcommand 'project' when required arguments are missing" in {
    testHelpOutput("update", Some("project"))
  }

  it should "display help for template subcommand 'add' when required arguments are missing" in {
    testHelpOutput("template", Some("add"))
  }

  it should "display help for template subcommand 'add-git' when required arguments are missing" in {
    testHelpOutput("template", Some("add-git"))
  }

  it should "include all subcommands in 'create' help text" in {
    val helpText = HelpTextManager.getCommandHelp("create")
    
    helpText should include("Command: create")
    helpText should include("DESCRIPTION:")
    helpText should include("USAGE:")
    helpText should include("SUBCOMMANDS:")
    helpText should include("project")
    helpText should include("default-templates")
    helpText should include("gcc-toolchain")
  }
  
  it should "list available subcommands in invalid subcommand error message" in {
    val helpText = HelpTextManager.getInvalidSubcommandHelp("create", "invalid")
    
    helpText should include("Error: Unknown subcommand 'invalid' for command 'create'")
    helpText should include("Available subcommands for 'create':")
    helpText should include("project")
    helpText should include("default-templates")
    helpText should include("gcc-toolchain")
  }  

}
