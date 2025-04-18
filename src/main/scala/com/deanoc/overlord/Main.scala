package com.deanoc.overlord

import com.deanoc.overlord.utils._
import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.cli.{CommandLineParser, CommandExecutor, Config}

import org.slf4j.event.Level
import scala.sys.process._
import scopt.OParser
import java.io.{ByteArrayOutputStream, PrintStream}
import com.deanoc.overlord.utils.ModuleLogger.exitApplication

/** Main entry point for the Overlord CLI.
  */
object Main extends Logging {

  /** Main method.
    *
    * @param args
    *   Command line arguments
    */
  def main(args: Array[String]): Unit = {
    // Simple CLI dispatch for help output
    args.toList match {
      case "help" :: cmd :: sub :: _ =>
        println(com.deanoc.overlord.cli.HelpTextManager.getSubcommandHelp(cmd, sub))
      case "help" :: cmd :: Nil =>
        println(com.deanoc.overlord.cli.HelpTextManager.getCommandHelp(cmd))
      case "help" :: Nil =>
        println(com.deanoc.overlord.cli.HelpTextManager.getGlobalHelp())
      case Nil =>
        println(com.deanoc.overlord.cli.HelpTextManager.getGlobalHelp())
      case _ =>        
        // Temporarily redirect System.err to suppress default error messages
        val originalErr = System.err
        val nullOutputStream = new java.io.OutputStream() {
          override def write(b: Int): Unit = {}
          override def write(b: Array[Byte]): Unit = {}
          override def write(b: Array[Byte], off: Int, len: Int): Unit = {}
        }
        val nullPrintStream = new java.io.PrintStream(nullOutputStream)
        System.setErr(nullPrintStream)
        
        try {
          com.deanoc.overlord.cli.CommandLineParser.parse(args) match {
            case Some(config) =>
              // Restore System.err before executing command
              System.setErr(originalErr)
              if (!com.deanoc.overlord.cli.CommandLineParser.validateAndDisplayHelp(config)) {
                sys.exit(1)
              }
              val success = com.deanoc.overlord.cli.CommandExecutor.execute(config)
              if (!success) sys.exit(1)
            case None =>
              // Command parsing failed, try to extract command and subcommand for focused help
              val partialConfig = extractPartialConfig(args)
              // Restore System.err before printing our custom help
              System.setErr(originalErr)
              println(com.deanoc.overlord.cli.HelpTextManager.getFocusedUsage(partialConfig))
              sys.exit(1)
          }
        } finally {
          // Ensure System.err is restored even if an exception occurs
          System.setErr(originalErr)
        }
    }
  }
  
  /** Extracts a partial Config from command line arguments when parsing fails.
    * This allows us to provide more focused help messages.
    *
    * @param args
    *   Command line arguments
    * @return
    *   A Config object with command and subCommand fields populated if possible
    */
  private def extractPartialConfig(args: Array[String]): Config = {
    val config = Config()
    
    if (args.length > 0) {
      // First argument is likely the command
      val command = args(0)
      val updatedConfig = config.copy(command = Some(command))
      
      if (args.length > 1) {
        // Second argument might be a subcommand
        val potentialSubcommand = args(1)
        // Only treat it as a subcommand if it doesn't start with a dash (which would indicate an option)
        if (!potentialSubcommand.startsWith("-")) {
          return updatedConfig.copy(subCommand = Some(potentialSubcommand))
        }
      }
      
      return updatedConfig
    }
    
    config
  }

  /** Configures logging based on the configuration.
    *
    * @param config
    *   The parsed configuration
    */
  private def configureLogging(config: Config): Unit = {
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
}
