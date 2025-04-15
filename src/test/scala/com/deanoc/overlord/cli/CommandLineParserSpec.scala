package com.deanoc.overlord.cli

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

class CommandLineParserSpec extends AnyFlatSpec with Matchers {

  "CommandLineParser" should "parse create project command" in {
    val args = Array("create", "project", "bare-metal", "my-project")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("create"))
    config.get.subCommand should be(Some("project"))
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
    val args = Array("generate", "report", "test.yaml")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("generate"))
    config.get.subCommand should be(Some("report"))
    config.get.infile should be(Some("test.yaml"))
  }

  it should "parse generate svd command" in {
    val args = Array("generate", "svd", "test.yaml")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("generate"))
    config.get.subCommand should be(Some("svd"))
    config.get.infile should be(Some("test.yaml"))
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
    val args = Array("update", "project", "test.yaml", "--instance", "cpu0")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("update"))
    config.get.subCommand should be(Some("project"))
    config.get.infile should be(Some("test.yaml"))
    config.get.instance should be(Some("cpu0"))
  }

  it should "parse update project command with yaml extension" in {
    val args = Array("update", "project", "test.yaml", "--instance", "cpu0")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("update"))
    config.get.subCommand should be(Some("project"))
    config.get.infile should be(Some("test.yaml"))
    config.get.instance should be(Some("cpu0"))
  }

  // Make sure the parser doesn't still reference .over files
  it should "parse generate commands with yaml extension consistently" in {
    val args = Array("generate", "svd", "system.yaml")
    val config = CommandLineParser.parse(args)

    config.isDefined should be(true)
    config.get.command should be(Some("generate"))
    config.get.subCommand should be(Some("svd"))
    config.get.infile should be(Some("system.yaml"))
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

  it should "successfully parse commands without the out option" in {
    val args = Array("create", "project", "template-name", "project-name")
    val config = CommandLineParser.parse(args)

    config shouldBe defined
    config.get.command shouldBe Some("create")
    config.get.subCommand shouldBe Some("project")
    config.get.templateName shouldBe Some("template-name")
    config.get.projectName shouldBe Some("project-name")
  }

  it should "parse generate commands with infile parameter" in {
    val args = Array("generate", "report", "project.yaml")
    val config = CommandLineParser.parse(args)

    config shouldBe defined
    config.get.command shouldBe Some("generate")
    config.get.subCommand shouldBe Some("report")
    config.get.infile shouldBe Some("project.yaml")
  }

  it should "parse update project command with infile parameter" in {
    val args = Array("update", "project", "project.yaml")
    val config = CommandLineParser.parse(args)

    config shouldBe defined
    config.get.command shouldBe Some("update")
    config.get.subCommand shouldBe Some("project")
    config.get.infile shouldBe Some("project.yaml")
  }
}
