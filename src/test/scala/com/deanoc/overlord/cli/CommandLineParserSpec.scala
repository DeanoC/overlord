package com.deanoc.overlord.cli

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

class CommandLineParserSpec extends AnyFlatSpec with Matchers {

  "CommandLineParser" should "parse create project command" in {
    val args = Array("create", "project", "test.over", "--board", "tangnano9k")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("create"))
    config.get.subCommand should be(Some("project"))
    config.get.infile should be(Some("test.over"))
    config.get.board should be(Some("tangnano9k"))
  }

  it should "parse create from-template command" in {
    val args = Array("create", "from-template", "bare-metal", "my-project")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("create"))
    config.get.subCommand should be(Some("from-template"))
    config.get.templateName should be(Some("bare-metal"))
    config.get.projectName should be(Some("my-project"))
  }

  it should "parse generate test command" in {
    val args = Array("generate", "test", "my-project")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("generate"))
    config.get.subCommand should be(Some("test"))
    config.get.projectName should be(Some("my-project"))
  }

  it should "parse generate report command" in {
    val args = Array("generate", "report", "test.over")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("generate"))
    config.get.subCommand should be(Some("report"))
    config.get.infile should be(Some("test.over"))
  }

  it should "parse generate svd command" in {
    val args = Array("generate", "svd", "test.over")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("generate"))
    config.get.subCommand should be(Some("svd"))
    config.get.infile should be(Some("test.over"))
  }

  it should "parse clean test command" in {
    val args = Array("clean", "test", "my-project")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("clean"))
    config.get.subCommand should be(Some("test"))
    config.get.projectName should be(Some("my-project"))
  }

  it should "parse update project command" in {
    val args = Array("update", "project", "test.over", "--instance", "cpu0")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("update"))
    config.get.subCommand should be(Some("project"))
    config.get.infile should be(Some("test.over"))
    config.get.instance should be(Some("cpu0"))
  }

  it should "parse update catalog command" in {
    val args = Array("update", "catalog")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("update"))
    config.get.subCommand should be(Some("catalog"))
  }

  it should "parse template list command" in {
    val args = Array("template", "list")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("list"))
  }

  it should "parse template add command" in {
    val args = Array("template", "add", "my-template", "/path/to/template")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("add"))
    config.get.templateName should be(Some("my-template"))
    config.get.options.get("path") should be(Some("/path/to/template"))
  }

  it should "parse template add-git command" in {
    val args = Array(
      "template",
      "add-git",
      "my-template",
      "https://github.com/example/template.git",
      "--branch",
      "main"
    )
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("add-git"))
    config.get.templateName should be(Some("my-template"))
    config.get.gitUrl should be(Some("https://github.com/example/template.git"))
    config.get.branch should be(Some("main"))
  }

  it should "parse template add-github command" in {
    val args = Array(
      "template",
      "add-github",
      "my-template",
      "example/template",
      "--ref",
      "v1.0.0"
    )
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("add-github"))
    config.get.templateName should be(Some("my-template"))
    config.get.ownerRepo should be(Some("example/template"))
    config.get.ref should be(Some("v1.0.0"))
  }

  it should "parse template remove command" in {
    val args = Array("template", "remove", "my-template")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("remove"))
    config.get.templateName should be(Some("my-template"))
  }

  it should "parse template update command" in {
    val args = Array("template", "update", "my-template")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("update"))
    config.get.templateName should be(Some("my-template"))
  }

  it should "parse template update-all command" in {
    val args = Array("template", "update-all")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("template"))
    config.get.subCommand should be(Some("update-all"))
  }

  it should "handle common options" in {
    val args = Array(
      "create",
      "project",
      "test.over",
      "--board",
      "tangnano9k",
      "--out",
      "output-dir",
      "--nostdresources",
      "--nostdprefabs",
      "--resources",
      "custom-resources",
      "--yes",
      "--noexit",
      "--trace",
      "Main,Resources",
      "--debug",
      "CommandLineParser",
      "--stdresource",
      "custom-std-resource"
    )
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("create"))
    config.get.subCommand should be(Some("project"))
    config.get.infile should be(Some("test.over"))
    config.get.board should be(Some("tangnano9k"))
    config.get.out should be("output-dir")
    config.get.nostdresources should be(true)
    config.get.nostdprefabs should be(true)
    config.get.resources should be(Some("custom-resources"))
    config.get.yes should be(true)
    config.get.noexit should be(true)
    config.get.trace should be(Some("Main,Resources"))
    config.get.debug should be(Some("CommandLineParser"))
    config.get.stdresource should be(Some("custom-std-resource"))
  }

  it should "return None for invalid commands" in {
    val args = Array("invalid", "command")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(false)
  }
}
