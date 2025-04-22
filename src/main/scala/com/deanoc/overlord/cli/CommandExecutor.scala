package com.deanoc.overlord.cli

import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.Component
import com.deanoc.overlord.DefinitionCatalog
import com.deanoc.overlord.cli.commands.CommandHandlerRegistry

import java.nio.file.{Files, Path, Paths}
import scala.sys.process._
import scopt.OParser
import scala.util.control.Breaks.{break, breakable}
import scala.collection._
import com.deanoc.overlord.definitions.DefinitionType
import com.deanoc.overlord.cli.commands.CommandHandler

/** Executes commands based on the parsed configuration.
  */
object CommandExecutor extends Logging {

  /** Helper method to extract common arguments from options map to dedicated
    * Config fields This ensures compatibility with code that expects dedicated
    * fields rather than options map entries
    */
  private def extractOptionsToFields(config: CliConfig): CliConfig = {
    // Create a copy of the config with fields populated from options map
    config.copy(
      templateName = config.options
        .get("template-name")
        .map(_.toString)
        .orElse(config.templateName),
      projectName = config.options
        .get("project-name")
        .map(_.toString)
        .orElse(config.projectName),
      inFile =
        config.options.get("infile").map(_.toString).orElse(config.inFile),
      boardName =
        config.options.get("board").map(_.toString).orElse(config.boardName),
      destination = config.options
        .get("destination")
        .map(_.toString)
        .orElse(config.destination),
      gccVersion = config.options
        .get("gcc-version")
        .map(_.toString)
        .orElse(config.gccVersion),
      binutilsVersion = config.options.get("binutils-version").map(_.toString)
      // Add other fields as needed
    )
  }

  /** Executes a command based on the configuration.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  def execute(config: CliConfig): Boolean = {
    // Extract commonly used arguments from options map for convenience
    val configWithExtractedOptions = extractOptionsToFields(config)

    (configWithExtractedOptions.command, configWithExtractedOptions.subCommand) match {
      // Check if we have a registered handler for this command + subcommand
      case (Some(command), Some(subcommand)) if CommandHandlerRegistry.handlerExists(command, subcommand) =>
        // Use a properly typed variable to fix the type inference issue
        val handlerOpt = CommandHandlerRegistry.getHandler(command, subcommand)
        handlerOpt match {
          case Some(handler) => 
            // Create a local variable with explicit type to help the compiler
            val typedHandler: CommandHandler = handler
            typedHandler.execute(configWithExtractedOptions)
          case None => 
            error(s"Internal error: Handler registry inconsistency for $command $subcommand")
            false
        }
        
      // Handle invalid subcommand for a valid command
      case (Some(command), Some(invalidSubcmd)) if CommandLineParser.commandExists(command) =>
        CommandLineParser.bufferedPrintln(
          HelpTextManager.getInvalidSubcommandHelp(command, invalidSubcmd)
        )
        false
        
      // Handle valid command with missing subcommand
      case (Some(cmd), None) if CommandLineParser.commandExists(cmd) =>
        CommandLineParser.bufferedPrintln(HelpTextManager.getCommandHelp(cmd))
        false

      // Handle HELP command separately
      case (Some("help"), _) =>
        val commandOpt = config.options.get("help-command").map(_.toString)
        val subcommandOpt =
          config.options.get("help-subcommand").map(_.toString)
        (commandOpt, subcommandOpt) match {
          case (None, _) =>
            val helpText = HelpTextManager.getGlobalHelp()
            print(helpText)
          case (Some(cmd), None) =>
            val helpText = HelpTextManager.getCommandHelp(cmd)
            print(helpText)
          case (Some(cmd), Some(sub)) =>
            val helpText = HelpTextManager.getSubcommandHelp(cmd, sub)
            print(helpText)
        }
        true

      // Handle invalid command
      case (Some(cmd), _) if !CommandLineParser.commandExists(cmd) =>
        println(HelpTextManager.getInvalidCommandHelp(cmd))
        false

      // Unknown command combination or missing subcommand/args
      case _ =>
        println(HelpTextManager.getFocusedUsage(config))
        false
    }
  }

  /** Loads a project from a file.
    *
    * @param config
    *   The parsed configuration
    * @param filename
    *   The file to load
    * @return
    *   The loaded project, or null if loading failed
    */
  def loadProject(config: CliConfig, filename: String): Overlord = {
    val expandedFilename = expandPath(filename)
    if (!Files.exists(Paths.get(expandedFilename))) {
      error(s"$expandedFilename does not exist")
      return null
    }

    val filePath = Paths.get(expandedFilename).toAbsolutePath.normalize()

    val parentDir = filePath.getParent.toAbsolutePath.normalize()
    Utils.ensureDirectories(parentDir)

    val board = config.boardName.getOrElse("unknown")

    Overlord(filename.split('/').last.split('.').head, board, filePath)
  }

  /** Expands a path, replacing ~ with the user's home directory.
    *
    * @param path
    *   The path to expand
    * @return
    *   The expanded path
    */
  def expandPath(path: String): String = {
    if (path.startsWith("~")) {
      path.replaceFirst("~", System.getProperty("user.home"))
    } else {
      path
    }
  }

}
