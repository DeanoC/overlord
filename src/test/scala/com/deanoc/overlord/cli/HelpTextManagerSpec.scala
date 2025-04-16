package com.deanoc.overlord.cli

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

class HelpTextManagerSpec extends AnyFlatSpec with Matchers {

  "HelpTextManager" should "provide global help text" in {
    val help = HelpTextManager.getGlobalHelp()
    help should include ("USAGE:")
    help should include ("COMMANDS:")
    help should include ("create")
    help should include ("help")
  }

  it should "provide command help for 'create'" in {
    val help = HelpTextManager.getCommandHelp("create")
    help should include ("Command: create")
    help should include ("Create a new project or from a template")
    help should include ("USAGE:")
    help should include ("overlord create [subcommand]")
    help should include ("EXAMPLES:")
  }

  it should "provide subcommand help for 'create project'" in {
    val help = HelpTextManager.getSubcommandHelp("create", "project")
    help should include ("Command: create project")
    help should include ("Create a new project from a .yaml file")
    help should include ("USAGE:")
    help should include ("overlord create project <infile>")
    help should include ("ARGUMENTS:")
    help should include ("OPTIONS:")
    help should include ("EXAMPLES:")
  }

  it should "provide subcommand help for 'create from-template'" in {
    val help = HelpTextManager.getSubcommandHelp("create", "from-template")
    help should include ("Command: create from-template")
    help should include ("Create a new project from a template")
    help should include ("overlord create from-template <template-name>")
    help should include ("ARGUMENTS:")
    help should include ("OPTIONS:")
    help should include ("EXAMPLES:")
  }

  it should "provide focused usage for command only" in {
    val config = Config(command = Some("create"))
    val usage = HelpTextManager.getFocusedUsage(config)
    usage should include ("USAGE:")
    usage should include ("overlord create [subcommand]")
    usage should include ("Run 'overlord help create'")
  }

  it should "provide focused usage for command and subcommand" in {
    val config = Config(command = Some("create"), subCommand = Some("project"))
    val usage = HelpTextManager.getFocusedUsage(config)
    usage should include ("USAGE:")
    usage should include ("overlord create project <infile>")
    usage should include ("ARGUMENTS:")
    usage should include ("OPTIONS:")
  }

  it should "fall back to global help for unknown command" in {
    val config = Config(command = Some("unknown"))
    val usage = HelpTextManager.getFocusedUsage(config)
    usage should include ("USAGE:")
    usage should include ("COMMANDS:")
  }
}