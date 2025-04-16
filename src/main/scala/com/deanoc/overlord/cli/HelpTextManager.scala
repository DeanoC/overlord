package com.deanoc.overlord.cli

/**
 * Data class for argument help.
 */
case class ArgumentHelp(
  name: String,
  description: String,
  required: Boolean
)

/**
 * Data class for option help.
 */
case class OptionHelp(
  name: String,
  shortName: Option[String],
  description: String,
  required: Boolean
)

/**
 * Data class for help text for a command or subcommand.
 */
case class HelpText(
  description: String,
  usage: String,
  arguments: List[ArgumentHelp] = Nil,
  options: List[OptionHelp] = Nil,
  examples: List[String] = Nil
) {
  def format(): String = {
    val sb = new StringBuilder
    sb.append("DESCRIPTION:\n  ").append(description.trim).append("\n\n")
    sb.append("USAGE:\n  ").append(usage).append("\n\n")
    if (arguments.nonEmpty) {
      sb.append("ARGUMENTS:\n")
      arguments.foreach { arg =>
        sb.append("  <" + arg.name + "> " + arg.description)
        if (arg.required) sb.append(" (required)")
        sb.append("\n")
      }
      sb.append("\n")
    }
    if (options.nonEmpty) {
      sb.append("OPTIONS:\n")
      options.foreach { opt =>
        val short = opt.shortName.map(s => s"-$s, ").getOrElse("")
        sb.append("  " + short + "--" + opt.name + " " + opt.description)
        if (opt.required) sb.append(" (required)")
        sb.append("\n")
      }
      sb.append("\n")
    }
    if (examples.nonEmpty) {
      sb.append("EXAMPLES:\n")
      examples.foreach { ex =>
        sb.append(s"  $ex\n")
      }
      sb.append("\n")
    }
    sb.toString
  }

  def formatConcise(helpHint: Option[String] = None): String = {
    val sb = new StringBuilder
    sb.append("USAGE:\n  ").append(usage).append("\n\n")
    if (arguments.nonEmpty) {
      sb.append("ARGUMENTS:\n")
      arguments.foreach { arg =>
        sb.append("  <" + arg.name + "> " + arg.description)
        if (arg.required) sb.append(" (required)")
        sb.append("\n")
      }
      sb.append("\n")
    }
    if (options.nonEmpty) {
      sb.append("OPTIONS:\n")
      options.foreach { opt =>
        val short = opt.shortName.map(s => s"-$s, ").getOrElse("")
        sb.append("  " + short + "--" + opt.name + " " + opt.description)
        if (opt.required) sb.append(" (required)")
        sb.append("\n")
      }
      sb.append("\n")
    }
    helpHint.foreach(hint => sb.append(hint).append("\n"))
    sb.toString
  }
}

/**
 * Manages help text for all commands and subcommands.
 */
object HelpTextManager {
  // Top-level commands and their help text
  private val globalHelp =
    """USAGE:
  overlord <command> [subcommand] [options]

COMMANDS:
  create     Create a new project or from a template
  generate   Generate various outputs (tests, reports, SVD files)
  clean      Clean generated files
  update     Update projects or catalog
  template   Manage project templates
  help       Display help for a specific command

Run 'overlord help <command>' for more information on a specific command.
"""

  // Map of command -> HelpText
  private val commandHelps: Map[String, HelpText] = Map(
    "create" -> HelpText(
      description =
        """Create a new project or from a template. This command allows you to create
projects either from scratch using a YAML configuration file or from
predefined templates.""",
      usage = "overlord create [subcommand] [options]",
      arguments = Nil,
      options = Nil,
      examples = List(
        "overlord create project my-project.yaml --board arty-a7",
        "overlord create from-template bare-metal my-new-project"
      )
    ),
    "generate" -> HelpText(
      description = "Generate various outputs (tests, reports, SVD files).",
      usage = "overlord generate <subcommand> [options]",
      arguments = Nil,
      options = Nil,
      examples = List(
        "overlord generate test my-project",
        "overlord generate report my-project.yaml"
      )
    ),
    "clean" -> HelpText(
      description = "Clean generated files.",
      usage = "overlord clean <subcommand> [options]",
      arguments = Nil,
      options = Nil,
      examples = List(
        "overlord clean test my-project"
      )
    ),
    "update" -> HelpText(
      description = "Update projects or catalog.",
      usage = "overlord update <subcommand> [options]",
      arguments = Nil,
      options = Nil,
      examples = List(
        "overlord update project my-project.yaml",
        "overlord update catalog"
      )
    ),
    "template" -> HelpText(
      description = "Manage project templates.",
      usage = "overlord template <subcommand> [options]",
      arguments = Nil,
      options = Nil,
      examples = List(
        "overlord template list",
        "overlord template add my-template ./path/to/template"
      )
    )
  )

  // Map of (command, subcommand) -> HelpText
  private val subcommandHelps: Map[(String, String), HelpText] = Map(
    ("create", "project") -> HelpText(
      description =
        """Create a new project from a .yaml file. This command creates a new project
directory using the specified YAML configuration file.""",
      usage = "overlord create project <infile> --board <board-name>",
      arguments = List(
        ArgumentHelp("infile", "Filename should be a .yaml file to use for the project", required = true)
      ),
      options = List(
        OptionHelp("board", None, "Board definition to use", required = true)
      ),
      examples = List(
        "overlord create project my-project.yaml --board arty-a7"
      )
    ),
    ("create", "from-template") -> HelpText(
      description =
        """Create a new project from a template. This command creates a new project
directory with files based on the selected template, customized with your
project name.""",
      usage = "overlord create from-template <template-name> <project-name> [options]",
      arguments = List(
        ArgumentHelp("template-name", "Name of the template to use (e.g., bare-metal, linux-app)", required = true),
        ArgumentHelp("project-name", "Name of the project to create", required = true)
      ),
      options = List(
        OptionHelp("yes", Some("y"), "Automatically agree to download resources if needed", required = false)
      ),
      examples = List(
        "overlord create from-template bare-metal my-project",
        "overlord create from-template linux-app my-linux-app --yes"
      )
    )
    // Add more subcommand help as needed
  )

  def getGlobalHelp(): String = globalHelp

  def getCommandHelp(command: String): String =
    commandHelps.get(command) match {
      case Some(help) => s"Command: $command\n\n" + help.format()
      case None       => s"No help available for command: $command"
    }

  def getSubcommandHelp(command: String, subcommand: String): String =
    subcommandHelps.get((command, subcommand)) match {
      case Some(help) => s"Command: $command $subcommand\n\n" + help.format()
      case None       => s"No help available for command: $command $subcommand"
    }

  /**
   * Returns focused usage for the given config (best effort).
   */
  def getFocusedUsage(config: Config): String = {
    (config.command, config.subCommand) match {
      case (Some(cmd), Some(sub)) =>
        subcommandHelps.get((cmd, sub)).map(_.formatConcise()).getOrElse(
          globalHelp
        )
      case (Some(cmd), None) =>
        commandHelps.get(cmd).map { help =>
          val hint = s"Run 'overlord help $cmd' for more detailed information."
          help.formatConcise(Some(hint))
        }.getOrElse(
          globalHelp
        )
      case _ =>
        globalHelp
    }
  }
}