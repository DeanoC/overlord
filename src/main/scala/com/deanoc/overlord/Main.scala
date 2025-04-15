package com.deanoc.overlord

import com.deanoc.overlord.utils._
import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.cli.{CommandLineParser, CommandExecutor}

import org.slf4j.event.Level
import scala.sys.process._
import scopt.OParser

/** Main entry point for the Overlord CLI.
  */
object Main extends Logging {

  /** Main method.
    *
    * @param args
    *   Command line arguments
    */
  def main(args: Array[String]): Unit = {
    // Parse command line arguments
    CommandLineParser.parse(args) match {
      case Some(config) =>
        // Configure ModuleLogger to exit on error (or not) based on command line option
        ModuleLogger.setExitOnError(!config.noexit)

        // Configure logging
        configureLogging(config)

        // Debug log the parsed configuration
        logConfigDetails(config)

        // Execute the command
        val success = CommandExecutor.execute(config)
        if (!success) {
          sys.exit(1)
        }

      case None =>
        // When parsing failed, show usage information and exit with an error code
        println(OParser.usage(CommandLineParser.createParser()))
        sys.exit(1)
    }
  }

  /** Configures logging based on the configuration.
    *
    * @param config
    *   The parsed configuration
    */
  private def configureLogging(config: com.deanoc.overlord.cli.Config): Unit = {
    // Helper function to process module names and set log levels
    def configureModuleLogLevels(modulesList: String, level: Level): Unit = {
      val moduleNames = modulesList.split(',').map(_.trim).filter(_.nonEmpty)

      moduleNames.foreach { moduleName =>
        // Handle various formats of module names
        val baseNames = if (moduleName.startsWith("com.")) {
          // Full package name - use as is
          Seq(moduleName)
        } else {
          // Simple name - just add the package prefix
          Seq(s"com.deanoc.overlord.$moduleName")
        }

        // For each potential base name, also handle both class and object forms
        val names = baseNames.flatMap(base => Seq(base, s"$base$$"))

        names.foreach { name =>
          info(s"Enabling ${level.name()} logging for module: $name")
          ModuleLogger.setModuleLogLevel(name, level)
        }
      }
    }

    // Set trace logging for specific modules if requested
    config.trace.foreach { traceModules =>
      configureModuleLogLevels(traceModules, Level.TRACE)
    }

    // Set debug logging for specific modules if requested
    config.debug.foreach { debugModules =>
      configureModuleLogLevels(debugModules, Level.DEBUG)
    }
  }

  /** Logs the details of the configuration for debugging.
    *
    * @param config
    *   The parsed configuration
    */
  private def logConfigDetails(config: com.deanoc.overlord.cli.Config): Unit = {
    debug("Configuration after parsing:")
    debug(s"- Command: ${config.command.getOrElse("None")}")
    debug(s"- Subcommand: ${config.subCommand.getOrElse("None")}")
    debug(s"- Output directory: Uses project file directory")
    debug(s"- Board: ${config.board.getOrElse("None")}")
    debug(s"- No standard resources: ${config.nostdresources}")
    debug(s"- No standard prefabs: ${config.nostdprefabs}")
    debug(s"- Resources path: ${config.resources.getOrElse("None")}")
    debug(s"- Instance: ${config.instance.getOrElse("None")}")
    debug(s"- Auto-yes: ${config.yes}")
    debug(s"- Input file: ${config.infile.getOrElse("None")}")
    debug(s"- Template name: ${config.templateName.getOrElse("None")}")
    debug(s"- Project name: ${config.projectName.getOrElse("None")}")
    debug(
      s"- Standard resource path: ${config.stdresource.getOrElse("Default")}"
    )
    debug(s"- No exit on error: ${config.noexit}")
    debug(s"- Trace modules: ${config.trace.getOrElse("None")}")
    debug(s"- Debug modules: ${config.debug.getOrElse("None")}")

    // Log git/GitHub options if present
    config.gitUrl.foreach(url => debug(s"- Git URL: $url"))
    config.branch.foreach(branch => debug(s"- Branch: $branch"))
    config.ownerRepo.foreach(ownerRepo => debug(s"- Owner/Repo: $ownerRepo"))
    config.ref.foreach(ref => debug(s"- Ref: $ref"))

    // Log additional options
    if (config.options.nonEmpty) {
      debug("- Additional options:")
      config.options.foreach { case (key, value) =>
        debug(s"  - $key: $value")
      }
    }
  }
}
