package com.deanoc.overlord

import com.deanoc.overlord.utils._
import com.deanoc.overlord.Project
import com.deanoc.overlord.utils.Logging

import java.nio.file.{Files, Path, Paths}
import scala.annotation.tailrec
import sys.process._
import scopt.OParser
import org.slf4j.event.Level

object Main extends Logging {
  case class Config(
      command: Option[String] = None,
      out: String = ".",
      board: Option[String] = None,
      nostdresources: Boolean = false,
      nostdprefabs: Boolean = false,
      resources: Option[String] = None,
      instance: Option[String] = None,
      yes: Boolean = false,
      infile: Option[String] = None,
      stdresource: Option[String] = None,
      noexit: Boolean = false,
      trace: Option[String] = None,
      debug: Option[String] = None
  )

  
  val parser: OParser[_, Config] = {
    val builder = OParser.builder[Config]
    import builder._
    OParser.sequence(
      programName("overlord"),
      head("overlord", "0.1.0"),
      cmd("create")
        .action((_, c) => c.copy(command = Some("create")))
        .text("generate a compile project with all sub parts at 'out'")
        .children(
          arg[String]("<infile>")
            .required()
            .action((x, c) => c.copy(infile = Some(x)))
            .text("filename should be a .over file to use for the project"),
          opt[String]("board")
            .required()
            .action((x, c) => c.copy(board = Some(x)))
            .text("board definition to use")
        ),
      cmd("update")
        .action((_, c) => c.copy(command = Some("update")))
        .text("update an existing project instance"),
      cmd("report")
        .action((_, c) => c.copy(command = Some("report")))
        .text("prints some info about the overall structure"),
      cmd("svd")
        .action((_, c) => c.copy(command = Some("svd")))
        .text("produce a CMSIS-SVD file on its own"),
      opt[String]("out")
        .action((x, c) => c.copy(out = x))
        .text("path where generated files should be placed"),
      opt[String]("board")
        .action((x, c) => c.copy(board = Some(x)))
        .text("board definition to use"),
      opt[Unit]("nostdresources")
        .action((_, c) => c.copy(nostdresources = true))
        .text("don't use the standard catalog"),
      opt[Unit]("nostdprefabs")
        .action((_, c) => c.copy(nostdprefabs = true))
        .text("don't use the standard prefabs"),
      opt[String]("resources")
        .action((x, c) => c.copy(resources = Some(x)))
        .text("use the specified path as the root of resources"),
      opt[String]("instance")
        .action((x, c) => c.copy(instance = Some(x)))
        .text("specify the instance to update"),
      opt[Unit]("yes")
        .abbr("y")
        .action((_, c) => c.copy(yes = true))
        .text(
          "automatically agree (e.g., download resource files without prompting)"
        ),
      arg[String]("<infile>")
        .optional()
        .action((x, c) => c.copy(infile = Some(x)))
        .text("filename should be a .over file to use for the project"),
      opt[Unit]("noexit")
        .action((_, c) => c.copy(noexit = true))
        .text("disable automatic exit on error logs"),
      opt[String]("trace")
        .action((x, c) => c.copy(trace = Some(x)))
        .text("enable trace logging for comma-separated list of modules (can use short names)"),
      opt[String]("debug")
        .action((x, c) => c.copy(debug = Some(x)))
        .text("enable debug logging for comma-separated list of modules (can use short names)"),
      opt[String]("stdresource")
        .action((x, c) => c.copy(stdresource = Some(x)))
        .text(s"specify the standard resource path {default: ${Resources.stdResourcePath()}}")
    )
  }

  def main(args: Array[String]): Unit = {
    // Parse arguments and set `yes` to true if running in a non-interactive environment
    val initialConfig = Config()
    val isNonInteractive = System.console() == null

    OParser.parse(parser, args, initialConfig.copy(yes = initialConfig.yes || isNonInteractive)) match {
      case Some(config) =>
        // Configure ModuleLogger to exit on error (or not) based on command line option
        ModuleLogger.setExitOnError(!config.noexit)
        
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
        
        // Set log levels first, before any logging happens
        // Set trace logging for specific modules if requested
        config.trace.foreach { traceModules =>
          configureModuleLogLevels(traceModules, Level.TRACE)
        }
        
        // Set debug logging for specific modules if requested
        config.debug.foreach { debugModules =>
          configureModuleLogLevels(debugModules, Level.DEBUG)
        }
               
        // Debug log the parsed configuration
        logConfigDetails(config)
        
        val usage = OParser.usage(parser) // Dynamically generate usage text
        val filename = config.infile.getOrElse {
          warn("Missing required input file. Please specify a .over file.")
          error(usage) // Show usage message
          sys.exit(1)
        }
        val expandedFilename = if (filename.startsWith("~")) {
          filename.replaceFirst("~", System.getProperty("user.home"))
        } else {
          filename
        }
        if (!Files.exists(Paths.get(expandedFilename))) {
          warn(s"$expandedFilename does not exist")
          error(usage)
          sys.exit(1)
        }

        val filePath = Paths.get(expandedFilename).toAbsolutePath.normalize()
        val parentDir = filePath.getParent.toAbsolutePath.normalize()

        // Change the current working directory to the parent directory of the .over file
        System.setProperty("user.dir", parentDir.toString)

        // Ensure the output directory is relative to the new working directory
        val expandedOut = if (config.out.startsWith("~")) {
          config.out.replaceFirst("~", System.getProperty("user.home"))
        } else {
          config.out
        }
        val out = Paths.get(expandedOut).toAbsolutePath.normalize()
        Utils.ensureDirectories(out)

        val stdResourcePath = config.stdresource
          .orElse(config.resources)
          .map(path => Paths.get(path).toAbsolutePath.normalize())
          .getOrElse(Resources.stdResourcePath())
        Resources.setStdResourcePath(stdResourcePath)

        Project.setupPaths(
          filePath.getParent,
          Resources.stdResourcePath(),
          Resources.stdResourcePath(),
          out
        )

        val resources = config.resources.map { path =>
          Resources(Paths.get(path))
        }

        def isValidGitRepo(path: Path): Boolean = {
          val gitDirPath = path.resolve(".git")
          Files.exists(gitDirPath) && Files.isDirectory(gitDirPath)
        }

        if (
          !config.nostdresources &&
          (!Files.exists(stdResourcePath) || !isValidGitRepo(stdResourcePath))
        ) {

          val repoExists = Files.exists(stdResourcePath)
          val message = if (repoExists) {
            s"Standard resource folder '$stdResourcePath' exists but is not a valid Git repository."
          } else {
            s"Standard resource folder '$stdResourcePath' does not exist."
          }
          info(message)

          val autoDownload = config.yes
          val shouldDownload = if (autoDownload) {
            info("Auto-downloading standard resource folder (-y/--yes specified)...")
            true
          } else {
            info("Waiting for user input about downloading resources")
            print(
              "Would you like to download the standard catalog from Git? (y/n): "
            )
            val response = scala.io.StdIn.readLine()
            info(s"User response for download: ${
                if (response == "y") "yes" else "no"
              }")
            response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
          }

          if (shouldDownload) {
            if (repoExists) {
              info("Removing invalid repository folder...")
              val removeCommand = s"rm -rf $stdResourcePath"
              val removeResult = removeCommand.!
              if (removeResult != 0) {
                error("Error: Failed to remove invalid repository folder.")
                sys.exit(1)
              }
            }

            // Ensure all parent directories for stdResourcePath exist
            val parentDir = stdResourcePath.getParent
            if (!Files.exists(parentDir)) {
              Files.createDirectories(parentDir)
            }

            info("Cloning standard resource folder from Git repository...")
            val cloneCommand =
              s"git clone https://github.com/DeanoC/gagameos_stdcatalog.git $stdResourcePath"
            val cloneResult = cloneCommand.!
            if (cloneResult != 0) {
              error("Error: Failed to clone the standard resource folder.")
              sys.exit(1)
            } else {
              info("Standard resource folder successfully downloaded.")

              // Initialize and update git submodules
              info("Initializing git submodules...")
              val initCommand = Process(
                Seq("git", "submodule", "init"),
                new java.io.File(stdResourcePath.toString)
              )
              val initResult = initCommand.!
              if (initResult != 0) {
                warn("Warning: Failed to initialize git submodules.")
              } else {
                info("Updating git submodules...")
                val updateCommand = Process(
                  Seq("git", "submodule", "update", "--recursive"),
                  new java.io.File(stdResourcePath.toString)
                )
                val updateResult = updateCommand.!
                if (updateResult != 0) {
                  warn("Warning: Failed to update git submodules.")
                } else {
                  info("Git submodules successfully initialized and updated.")
                }
              }
            }
          } else {
            warn(
              "Download skipped. The standard catalog will not be available."
            )
            error(
              "Exiting as standard resources are required. Use --nostdresources to proceed without them."
            )
            sys.exit(1)
          }
        }

        val chipCatalog = new DefinitionCatalog
        val stdResources = if (!config.nostdresources) {
          val res = Resources(stdResourcePath)
          chipCatalog.mergeNewDefinition(res.loadCatalogs())
          res
        } else {
          null
        }
        resources.foreach(r => chipCatalog.mergeNewDefinition(r.loadCatalogs()))

        val prefabCatalog = new PrefabCatalog
        if (!config.nostdprefabs && stdResources != null) {
          prefabCatalog.prefabs ++= stdResources.loadPrefabs()
        }
        resources.foreach(r => prefabCatalog.prefabs ++= r.loadPrefabs())

        val gameName = filename.split('/').last.split('.').head
        val game = Project(
          gameName,
          config.board.get,
          filePath,
          chipCatalog,
          prefabCatalog
        ) match {
          case Some(game) => game
          case None       => return
        }

        config.command match {
          case Some("create") =>
            output.Project(game)
            info(s"** Project created at $out **")
          case Some("update") =>
            output.UpdateProject(game, config.instance)
          case _ =>
            if (config.command.contains("report")) output.Report(game)
            if (config.command.contains("svd")) output.Svd(game)
        }

      case None =>
        error(OParser.usage(parser)) // Show dynamically generated usage message
        sys.exit(1)
    }
  }
  
  // Helper method to log the details of the configuration for debugging
  private def logConfigDetails(config: Config): Unit = {
    debug("Configuration after parsing:")
    debug(s"- Command: ${config.command.getOrElse("None")}")
    debug(s"- Output directory: ${config.out}")
    debug(s"- Board: ${config.board.getOrElse("None")}")
    debug(s"- No standard resources: ${config.nostdresources}")
    debug(s"- No standard prefabs: ${config.nostdprefabs}")
    debug(s"- Resources path: ${config.resources.getOrElse("None")}")
    debug(s"- Instance: ${config.instance.getOrElse("None")}")
    debug(s"- Auto-yes: ${config.yes}")
    debug(s"- Input file: ${config.infile.getOrElse("None")}")
    debug(s"- Standard resource path: ${config.stdresource.getOrElse("Default")}")
    debug(s"- No exit on error: ${config.noexit}")
    debug(s"- Trace modules: ${config.trace.getOrElse("None")}")
    debug(s"- Debug modules: ${config.debug.getOrElse("None")}")
  }
}
