package com.deanoc.overlord.cli

import com.deanoc.overlord.utils.Logging

// --- CLI Metadata Model ---

sealed trait CLIElement
case class CLICommand(
  name: String,
  description: String,
  longDescription: Option[String] = None,
  subcommands: List[CLISubcommand] = Nil,
  options: List[CLIOption] = Nil,
  arguments: List[CLIArgument] = Nil,
  examples: List[String] = Nil
) extends CLIElement

case class CLISubcommand(
  name: String,
  description: String,
  longDescription: Option[String] = None,
  options: List[CLIOption] = Nil,
  arguments: List[CLIArgument] = Nil,
  examples: List[String] = Nil
) extends CLIElement

case class CLIOption(
  name: String,
  short: Option[String],
  description: String,
  required: Boolean = false
) extends CLIElement

case class CLIArgument(
  name: String,
  description: String,
  required: Boolean = true
) extends CLIElement

/** Parser for the Overlord CLI commands. Implements a hierarchical command
  * structure similar to git.
  */
object CommandLineParser extends Logging {

  import scopt.OParser

  // Common/global options available to all commands
  val commonOptions: List[CLIOption] = List(
    CLIOption("yes", Some("y"), "Automatically agree (e.g., download resource files without prompting)", required = false),
    CLIOption("noexit", None, "Disable automatic exit on error logs", required = false),
    CLIOption("trace", None, "Enable trace logging for comma-separated list of modules (can use short names)", required = false),
    CLIOption("debug", None, "Enable debug logging for comma-separated list of modules (can use short names)", required = false)
  )

  // Generate scopt OParser from CLI metadata
  def toOParser: OParser[_, Config] = {
    val builder = scopt.OParser.builder[Config]
    import builder._

    // Helper to convert CLIOption to scopt opt
    def optDef(opt: CLIOption) = {
      val base = builder.opt[String](opt.name)
        .optional()
        .text(opt.description)
      val withShort = opt.short match {
        case Some(s) => base.abbr(s)
        case None    => base
      }
      withShort
    }

    // Build a flat command structure with explicit command and subcommand handling
    val commandParsers = allCommandMetas.map { cmdMeta =>
      val cmdParser = cmd(cmdMeta.name)
        .text(cmdMeta.description)
        .action((_, c) => c.copy(command = Some(cmdMeta.name)))
        .children(
          cmdMeta.arguments.map { arg =>
            val baseArg = if (arg.required)
              builder.arg[String](s"<${arg.name}>").required().text(arg.description)
            else
              builder.arg[String](s"<${arg.name}>").optional().text(arg.description)
            
            baseArg.action((value, config) =>
              config.copy(options = config.options.concat(Map(arg.name -> value)))
            )
          } ++
          cmdMeta.options.map(optDef) ++
          commonOptions.map(optDef) ++
          cmdMeta.subcommands.map { sub =>
            cmd(sub.name)
              .text(sub.description)
              .action((_, c) => c.copy(command = Some(cmdMeta.name), subCommand = Some(sub.name)))
              .children(
                sub.arguments.map { arg =>
                  val baseArg = if (arg.required)
                    builder.arg[String](s"<${arg.name}>").required().text(arg.description)
                  else
                    builder.arg[String](s"<${arg.name}>").optional().text(arg.description)
                  
                  baseArg.action((value, config) =>
                    config.copy(options = config.options.concat(Map(arg.name -> value)))
                  )
                } ++
                sub.options.map(optDef) ++
                commonOptions.map(optDef): _*
              )
          }: _*
        )
      cmdParser
    }

    OParser.sequence(
      programName("overlord"),
      commandParsers: _*
    )
  }

  // Parse command line arguments using the generated OParser
  def parse(args: Array[String]): Option[Config] = {
    // Debug parsing attempt
    trace(s"Processing command: ${args.mkString(" ")}")
    
    // Special handling for subcommands with missing arguments
    if (args.length >= 2) {
      val cmd = args(0)
      val subcmd = args(1)
      
      if (commandExists(cmd) && subcommandExists(cmd, subcmd)) {
        // It's a valid command and subcommand - create a partial config
        val partialConfig = Config(command = Some(cmd), subCommand = Some(subcmd))
        
        // Further args would be processed as options/arguments
        if (args.length > 2) {
          // Has additional args - let scopt process as normal
          val result = OParser.parse(toOParser, args, Config())
          if (result.isDefined) return result
        }
        
        // No additional args or parsing failed - return partial config to show targeted help
        return Some(partialConfig)
      }
    }
    
    // Use the scopt OParser to parse the arguments normally
    val result = OParser.parse(toOParser, args, Config())
    
    result.flatMap { config =>
      trace(s"Parsing partial config: $config")
      
      // Validate the parsed configuration
      if (config.command.isDefined) {
        val cmd = config.command.get
        
        if (config.subCommand.isDefined) {
          val subcmd = config.subCommand.get
          
          if (commandExists(cmd) && subcommandExists(cmd, subcmd)) {
            // Validate required arguments for the subcommand
            val (requiredArgs, _) = getArgsAndOptions(cmd, Some(subcmd))
            validateRequiredArguments(config, requiredArgs) match {
              case Some(errorMsg) =>
                trace(errorMsg)
                None
              case None => Some(config)
            }
          } else {
            trace(s"Invalid command/subcommand combination: $cmd $subcmd")
            None
          }
        } else {
          // Validate required arguments for the command
          val (requiredArgs, _) = getArgsAndOptions(cmd)
          validateRequiredArguments(config, requiredArgs) match {
            case Some(errorMsg) =>
              trace(errorMsg)
              None
            case None => Some(config)
          }
        }
      } else {
        Some(config)  // No command specified, return as is
      }
    }
  }
  
  // Validate that all required arguments are present
  def validateRequiredArguments(config: Config, args: List[CLIArgument]): Option[String] = {
    val requiredArgs = args.filter(_.required)
    val missingArgs = requiredArgs.filterNot(arg => config.options.contains(arg.name))
    
    if (missingArgs.nonEmpty) {
      val msgList = missingArgs.map(arg => s"Missing argument <${arg.name}>")
      Some(msgList.mkString("\n"))
    } else {
      None
    }
  }

  // "create" command and subcommands
  val createCommandMeta = CLICommand(
    name = "create",
    description = "Create a new project or from a template.",
    longDescription = Some(
      """Create a new project or from a template. This command allows you to create
projects either from scratch using a YAML configuration file or from
predefined templates."""
    ),
    subcommands = List(
      CLISubcommand(
        name = "project",
        description = "Create a new project from a template",
        longDescription = Some(
          """Create a new project from a .yaml file. This command creates a new project
directory using the specified YAML configuration file."""
        ),
        arguments = List(
          CLIArgument("template-name", "name of the template to use (e.g., bare-metal, linux-app)", required = true),
          CLIArgument("project-name", "name of the project to create (will be created in the same directory as the project YAML file)", required = true)
        ),
        examples = List("overlord create project my-project.yaml --board arty-a7")
      ),
      CLISubcommand(
        name = "default-templates",
        description = "Download standard templates without creating a project",
        examples = List("overlord create default-templates")
      ),
      CLISubcommand(
        name = "gcc-toolchain",
        description = "Create a GCC cross-compilation toolchain",
        longDescription = Some("Create a GCC cross-compilation toolchain."),
        arguments = List(
          CLIArgument("triple", "target triple (e.g., arm-none-eabi, riscv64-unknown-elf)", required = true),
          CLIArgument("destination", "directory where the toolchain will be installed", required = true)
        ),
        options = List(
          CLIOption("gcc-version", None, "GCC version to use (default: 10.2.0)", required = false),
          CLIOption("binutils-version", None, "binutils version to use (default: 2.35)", required = false)
        ),
        examples = List("overlord create gcc-toolchain arm-none-eabi /opt/toolchains/arm --gcc-version 10.2.0 --binutils-version 2.35")
      )
    ),
    examples = List(
      "overlord create project my-project.yaml --board arty-a7",
      "overlord create from-template bare-metal my-new-project"
    )
  )

  // "generate" command and subcommands
  val generateCommandMeta = CLICommand(
    name = "generate",
    description = "Generate various outputs (tests, reports, SVD files).",
    subcommands = List(
      CLISubcommand(
        name = "test",
        description = "Generate test files for a project",
        arguments = List(
          CLIArgument("project-name", "name of the project to generate tests for", required = true)
        )
      ),
      CLISubcommand(
        name = "report",
        description = "Generate a report about the project structure",
        arguments = List(
          CLIArgument("infile", "filename should be a .yaml file to use for the project", required = true)
        )
      ),
      CLISubcommand(
        name = "svd",
        description = "Generate a CMSIS-SVD file",
        arguments = List(
          CLIArgument("infile", "filename should be a .yaml file to use for the project", required = true)
        )
      )
    )
  )

  // "clean" command and subcommands
  val cleanCommandMeta = CLICommand(
    name = "clean",
    description = "Clean generated files.",
    subcommands = List(
      CLISubcommand(
        name = "test",
        description = "Clean test files for a project",
        arguments = List(
          CLIArgument("project-name", "name of the project to clean tests for", required = true)
        )
      )
    )
  )

  // "update" command and subcommands
  val updateCommandMeta = CLICommand(
    name = "update",
    description = "Update projects or catalog.",
    subcommands = List(
      CLISubcommand(
        name = "project",
        description = "Update an existing project",
        arguments = List(
          CLIArgument("infile", "filename should be a .yaml file to use for the project", required = true)
        ),
        options = List(
          CLIOption("instance", None, "specify the instance to update", required = false)
        )
      ),
      CLISubcommand(
        name = "catalog",
        description = "Update the catalog from the remote repository"
      )
    )
  )

  // "template" command and subcommands
  val templateCommandMeta = CLICommand(
    name = "template",
    description = "Manage project templates.",
    subcommands = List(
      CLISubcommand(
        name = "list",
        description = "List all available templates"
      ),
      CLISubcommand(
        name = "add",
        description = "Add a local template",
        arguments = List(
          CLIArgument("name", "name of the template", required = true),
          CLIArgument("path", "path to the template directory", required = true)
        )
      ),
      CLISubcommand(
        name = "add-git",
        description = "Add a template from a git repository",
        arguments = List(
          CLIArgument("name", "name of the template", required = true),
          CLIArgument("git-url", "URL of the git repository", required = true)
        ),
        options = List(
          CLIOption("branch", None, "branch to use (default: main)", required = false)
        )
      ),
      CLISubcommand(
        name = "add-github",
        description = "Add a template from GitHub",
        arguments = List(
          CLIArgument("name", "name of the template", required = true),
          CLIArgument("owner/repo", "GitHub repository in the format 'owner/repo'", required = true)
        ),
        options = List(
          CLIOption("ref", None, "reference to use (tag, branch, or commit hash, default: main)", required = false)
        )
      ),
      CLISubcommand(
        name = "remove",
        description = "Remove a template",
        arguments = List(
          CLIArgument("name", "name of the template to remove", required = true)
        )
      ),
      CLISubcommand(
        name = "update",
        description = "Update a template from its source",
        arguments = List(
          CLIArgument("name", "name of the template to update", required = true)
        )
      ),
      CLISubcommand(
        name = "update-all",
        description = "Update all templates from their sources"
      )
    )
  )

  // "help" command
  val helpCommandMeta = CLICommand(
    name = "help",
    description = "Display help for a specific command",
    arguments = List(
      CLIArgument("command", "Command to get help for", required = false),
      CLIArgument("subcommand", "Subcommand to get help for", required = false)
    )
  )

  // List of all top-level commands (metadata)
  val allCommandMetas: List[CLICommand] = List(
    createCommandMeta,
    generateCommandMeta,
    cleanCommandMeta,
    updateCommandMeta,
    templateCommandMeta,
    helpCommandMeta
  )

  // --- Metadata-based utility functions for help system ---

  def getAllCommands: List[(String, String)] =
    allCommandMetas.map(cmd => (cmd.name, cmd.description))

  def getSubcommandsFor(command: String): List[(String, String)] =
    allCommandMetas.find(_.name == command).map(_.subcommands.map(sc => (sc.name, sc.description))).getOrElse(Nil)

  def getArgsAndOptions(command: String, subcommand: Option[String] = None): (List[CLIArgument], List[CLIOption]) = {
    allCommandMetas.find(_.name == command) match {
      case Some(cmdMeta) =>
        subcommand match {
          case Some(sub) =>
            cmdMeta.subcommands.find(_.name == sub) match {
              case Some(subMeta) =>
                (subMeta.arguments, subMeta.options)
              case None => (Nil, Nil)
            }
          case None =>
            (cmdMeta.arguments, cmdMeta.options)
        }
      case None => (Nil, Nil)
    }
  }
  
  // Check if a command exists
  def commandExists(command: String): Boolean = {
    allCommandMetas.exists(_.name == command)
  }
  
  // Check if a subcommand exists for a given command
  def subcommandExists(command: String, subcommand: String): Boolean = {
    allCommandMetas.find(_.name == command) match {
      case Some(cmdMeta) => cmdMeta.subcommands.exists(_.name == subcommand)
      case None => false
    }
  }
  
  // Find the closest matching subcommand for a given command
  def findClosestSubcommand(command: String, subcommand: String): Option[String] = {
    allCommandMetas.find(_.name == command) match {
      case Some(cmdMeta) =>
        val subcommands = cmdMeta.subcommands.map(_.name)
        if (subcommands.isEmpty) {
          None
        } else {
          // Find the closest match using Levenshtein distance
          val distances = subcommands.map(s => (s, levenshteinDistance(s, subcommand)))
          val closest = distances.minBy(_._2)
          // Only suggest if the distance is reasonable (less than half the length of the subcommand)
          if (closest._2 <= subcommand.length / 2) {
            Some(closest._1)
          } else {
            None
          }
        }
      case None => None
    }
  }
  
  // Calculate Levenshtein distance between two strings
  private def levenshteinDistance(s1: String, s2: String): Int = {
    val dist = Array.tabulate(s2.length + 1, s1.length + 1) { (j, i) =>
      if (j == 0) i else if (i == 0) j else 0
    }
    
    for (j <- 1 to s2.length; i <- 1 to s1.length) {
      dist(j)(i) = if (s2(j - 1) == s1(i - 1)) {
        dist(j - 1)(i - 1)
      } else {
        math.min(math.min(dist(j - 1)(i) + 1, dist(j)(i - 1) + 1), dist(j - 1)(i - 1) + 1)
      }
    }
    
    dist(s2.length)(s1.length)
  }

  /**
   * Validates the configuration and displays help or errors if needed.
   * This method should be called before `CommandExecutor.execute`.
   *
   * @param config The parsed configuration
   * @param displayErrorMessages Whether to display error messages about missing arguments (true for CLI, false for tests)
   * @return true if the configuration is valid, false otherwise
   */
  def validateAndDisplayHelp(config: Config, displayErrorMessages: Boolean = true): Boolean = {
    (config.command, config.subCommand) match {
      case (Some(cmd), Some(subcmd)) if commandExists(cmd) && subcommandExists(cmd, subcmd) =>
        val (requiredArgs, _) = getArgsAndOptions(cmd, Some(subcmd))
        validateRequiredArguments(config, requiredArgs) match {
          case Some(errorMsg) if displayErrorMessages =>
            // Print the error messages about missing arguments - but only in CLI mode
            errorMsg.split("\n").foreach(bufferedPrintln)
            
            // Display focused help for just this subcommand
            bufferedPrintln(HelpTextManager.getSubcommandHelp(cmd, subcmd))
            false
          case Some(_) =>
            // In test mode, don't display error messages, just the help
            bufferedPrintln(HelpTextManager.getSubcommandHelp(cmd, subcmd))
            false
          case None => true
        }

      case (Some(cmd), None) if commandExists(cmd) =>
        val cmdMeta = allCommandMetas.find(_.name == cmd).get
        if (cmdMeta.subcommands.nonEmpty) {
          // Command requires a subcommand but none was supplied
          bufferedPrintln(HelpTextManager.getCommandHelp(cmd))
          false
        } else {
          val (requiredArgs, _) = getArgsAndOptions(cmd)
          validateRequiredArguments(config, requiredArgs) match {
            case Some(errorMsg) =>
              // Print the error messages about missing arguments
              errorMsg.split("\n").foreach(bufferedPrintln)
              // Then display only the relevant command help
              bufferedPrintln(HelpTextManager.getCommandHelp(cmd))
              false
            case None => true
          }
        }

      case (Some(cmd), _) if !commandExists(cmd) =>
        bufferedPrintln(HelpTextManager.getInvalidCommandHelp(cmd))
        false

      case (Some(cmd), Some(subcmd)) =>
        bufferedPrintln(HelpTextManager.getInvalidSubcommandHelp(cmd, subcmd))
        false

      case (Some("help"), _) =>
        bufferedPrintln(HelpTextManager.getGlobalHelp())
        false

      case _ =>
        bufferedPrintln(HelpTextManager.getGlobalHelp())
        false
    }
  }

  private var printBuffer: StringBuilder = new StringBuilder

  /**
   * Custom print function that captures output to a buffer and also prints to the terminal.
   */
  def bufferedPrintln(message: String): Unit = {
    printBuffer.append(message).append("\n")
    println(message)
  }

  /**
   * Retrieve the current contents of the print buffer.
   */
  def getPrintBuffer: String = printBuffer.toString

  /**
   * Clear the print buffer.
   */
  def clearPrintBuffer(): Unit = {
    printBuffer.clear()
  }
}
