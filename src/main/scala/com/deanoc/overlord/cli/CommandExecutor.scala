package com.deanoc.overlord.cli

import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.DefinitionCatalog
import com.deanoc.overlord.templates.TemplateManager
import com.deanoc.overlord.test.TestManager
import com.deanoc.overlord.catalog.CatalogManager
import com.deanoc.overlord.output

import java.nio.file.{Files, Path, Paths}
import scala.sys.process._
import scopt.OParser
import scala.util.control.Breaks.{break, breakable}

/** Executes commands based on the parsed configuration.
  */
object CommandExecutor extends Logging {

  /** Helper method to extract common arguments from options map to dedicated Config fields
    * This ensures compatibility with code that expects dedicated fields rather than options map entries
    */
  private def extractOptionsToFields(config: Config): Config = {
    // Create a copy of the config with fields populated from options map
    config.copy(
      templateName = config.options.get("template-name").map(_.toString).orElse(config.templateName),
      projectName = config.options.get("project-name").map(_.toString).orElse(config.projectName),
      inFile = config.options.get("infile").map(_.toString).orElse(config.inFile),
      boardName = config.options.get("board").map(_.toString).orElse(config.boardName),
      destination = config.options.get("destination").map(_.toString).orElse(config.destination),
      gccVersion = config.options.get("gcc-version").map(_.toString).orElse(config.gccVersion),
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
  def execute(config: Config): Boolean = {
    // Extract commonly used arguments from options map for convenience
    val configWithExtractedOptions = extractOptionsToFields(config)
    
    (configWithExtractedOptions.command, configWithExtractedOptions.subCommand) match {
      // CREATE commands
      case (Some("create"), Some("project")) =>
        executeCreateProject(configWithExtractedOptions)

      case (Some("create"), Some("default-templates")) =>
        executeCreateDefaultTemplates(configWithExtractedOptions)

      case (Some("create"), Some("gcc-toolchain")) =>
        executeCreateGccToolchain(configWithExtractedOptions)
        
      // Handle invalid subcommand for create
      case (Some("create"), Some(invalidSubcmd)) =>
        CommandLineParser.bufferedPrintln(HelpTextManager.getInvalidSubcommandHelp("create", invalidSubcmd))
        false

      // GENERATE commands
      case (Some("generate"), Some("test")) =>
        executeGenerateTest(configWithExtractedOptions)

      case (Some("generate"), Some("report")) =>
        executeGenerateReport(configWithExtractedOptions)

      case (Some("generate"), Some("svd")) =>
        executeGenerateSvd(configWithExtractedOptions)
        
      // Handle invalid subcommand for generate
      case (Some("generate"), Some(invalidSubcmd)) =>
        CommandLineParser.bufferedPrintln(HelpTextManager.getInvalidSubcommandHelp("generate", invalidSubcmd))
        false

      // CLEAN commands
      case (Some("clean"), Some("test")) =>
        executeCleanTest(configWithExtractedOptions)
        
      // Handle invalid subcommand for clean
      case (Some("clean"), Some(invalidSubcmd)) =>
        val helpText = HelpTextManager.getInvalidSubcommandHelp("clean", invalidSubcmd)
        print(helpText)
        false

      // UPDATE commands
      case (Some("update"), Some("project")) =>
        executeUpdateProject(configWithExtractedOptions)

      case (Some("update"), Some("catalog")) =>
        executeUpdateCatalog(configWithExtractedOptions)
        
      // Handle invalid subcommand for update
      case (Some("update"), Some(invalidSubcmd)) =>
        val helpText = HelpTextManager.getInvalidSubcommandHelp("update", invalidSubcmd)
        print(helpText)
        false

      // TEMPLATE commands
      case (Some("template"), Some("list")) =>
        executeTemplateList(configWithExtractedOptions)

      case (Some("template"), Some("add")) =>
        executeTemplateAdd(configWithExtractedOptions)

      case (Some("template"), Some("add-git")) =>
        executeTemplateAddGit(configWithExtractedOptions)

      case (Some("template"), Some("add-github")) =>
        executeTemplateAddGitHub(configWithExtractedOptions)

      case (Some("template"), Some("remove")) =>
        executeTemplateRemove(configWithExtractedOptions)

      case (Some("template"), Some("update")) =>
        executeTemplateUpdate(configWithExtractedOptions)

      case (Some("template"), Some("update-all")) =>
        executeTemplateUpdateAll(configWithExtractedOptions)
        
      // Handle invalid subcommand for template
      case (Some("template"), Some(invalidSubcmd)) =>
        val helpText = HelpTextManager.getInvalidSubcommandHelp("template", invalidSubcmd)
        print(helpText)
        false

      // HELP command
      case (Some("help"), _) =>
        val commandOpt = config.options.get("help-command").map(_.toString)
        val subcommandOpt = config.options.get("help-subcommand").map(_.toString)
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

      // Handle valid command with missing subcommand
      case (Some(cmd), None) if CommandLineParser.commandExists(cmd) =>
        CommandLineParser.bufferedPrintln(HelpTextManager.getCommandHelp(cmd))
        false
        
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

  /** Executes the 'create project' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeCreateProject(
      config: Config
  ): Boolean = {
    val templateNameOpt = config.templateName
    val projectNameOpt = config.projectName

    (templateNameOpt, projectNameOpt) match {
      case (Some(templateName), Some(projectName)) =>
        // Check if any templates are available
        if (!TemplateManager.hasTemplates()) {
          info("No templates available")

          // Ask if the user wants to download standard templates
          val shouldDownload = if (config.yes) {
            info("Auto-downloading standard templates (-y/--yes specified)...")
            true
          } else {
            info("Waiting for user input about downloading templates")
            print(
              "No templates are installed. Would you like to download a standard set of templates from GitHub? (y/n): "
            )
            val response = scala.io.StdIn.readLine()
            info(s"User response for download: ${
                if (response == "y") "yes" else "no"
              }")
            response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
          }

          if (shouldDownload) {
            if (!TemplateManager.downloadStandardTemplates(config.yes)) {
              warn("Failed to download standard templates")
              return false
            }
          } else {
            info("Template download skipped")
            return false
          }
        }

        // Now try to create the project from the template
        TemplateManager.createFromTemplate(
          templateName,
          projectName,
          "." // Use current directory as output path
        )
      case (None, _) =>
        error("Missing required template name")
        false
      case (_, None) =>
        error("Missing required project name")
        false
    }
  }

  /** Executes the 'generate test' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeGenerateTest(config: Config): Boolean = {
    config.projectName.fold {
      error("Missing required project name")
      false
    } { projectName =>
      TestManager.generateTests(projectName)
    }
  }

  /** Executes the 'generate report' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeGenerateReport(config: Config): Boolean = {
    config.inFile match {
      case Some(filename) =>
        try {
          val game = loadProject(config, filename)
          if (game != null) {
            output.Report(game)
            true
          } else {
            false
          }
        } catch {
          case e: Exception =>
            error(s"Error generating report: ${e.getMessage}")
            false
        }
      case None =>
        error("Missing required input file")
        false
    }
  }

  /** Executes the 'generate svd' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeGenerateSvd(config: Config): Boolean = {
    config.inFile match {
      case Some(filename) =>
        try {
          val game = loadProject(config, filename)
          if (game != null) {
            output.Svd(game)
            true
          } else {
            false
          }
        } catch {
          case e: Exception =>
            error(s"Error generating SVD: ${e.getMessage}")
            false
        }
      case None =>
        error("Missing required input file")
        false
    }
  }

  /** Executes the 'clean test' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeCleanTest(config: Config): Boolean = {
    config.projectName.fold {
      error("Missing required project name")
      false
    } { projectName =>
      TestManager.cleanTests(projectName)
    }
  }

  /** Executes the 'update project' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeUpdateProject(config: Config): Boolean = {
    config.inFile match {
      case Some(filename) =>
        try {
          val game = loadProject(config, filename)
          if (game != null) {
            val instance = config.options.get("instance").map(_.toString)
            output.UpdateProject(game, instance)
            true
          } else {
            false
          }
        } catch {
          case e: Exception =>
            error(s"Error updating project: ${e.getMessage}")
            false
        }
      case None =>
        error("Missing required input file")
        false
    }
  }

  /** Executes the 'update catalog' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeUpdateCatalog(config: Config): Boolean = {
    CatalogManager.updateCatalog()
  }

  /** Executes the 'template list' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateList(config: Config): Boolean = {
    val templates = TemplateManager.listAvailableTemplates()

    if (templates.isEmpty) {
      info("No templates available")

      // Ask if the user wants to download standard templates
      val shouldDownload = if (config.yes) {
        info("Auto-downloading standard templates (-y/--yes specified)...")
        true
      } else {
        info("Waiting for user input about downloading templates")
        print(
          "Would you like to download a standard set of templates from GitHub? (y/n): "
        )
        val response = scala.io.StdIn.readLine()
        info(
          s"User response for download: ${if (response == "y") "yes" else "no"}"
        )
        response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
      }

      if (shouldDownload) {
        if (TemplateManager.downloadStandardTemplates(config.yes)) {
          // List templates again after downloading
          val updatedTemplates = TemplateManager.listAvailableTemplates()
          if (updatedTemplates.nonEmpty) {
            info("Available templates:")
            updatedTemplates.foreach { template =>
              info(s"  $template")
            }
          } else {
            warn("Failed to download templates or no templates were downloaded")
          }
        } else {
          warn("Failed to download standard templates")
        }
      } else {
        info("Template download skipped")
      }
    } else {
      info("Available templates:")
      templates.foreach { template =>
        info(s"  $template")
      }
    }

    true
  }

  /** Executes the 'template add' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateAdd(config: Config): Boolean = {
    val nameOpt = config.templateName
    val pathOpt =
      config.options.get("path").collect { case s: String if s.nonEmpty => s }

    (nameOpt, pathOpt) match {
      case (Some(name), Some(path)) =>
        // This would call the TemplateManager.addLocalTemplate method
        // For now, we'll just log the action
        info(s"Adding local template '$name' from path '$path'")
        true
      case (None, _) =>
        error("Missing required template name")
        false
      case (_, None) =>
        error("Missing required path")
        false
    }
  }

  /** Executes the 'template add-git' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateAddGit(config: Config): Boolean = {
    val nameOpt = config.templateName
    val gitUrlOpt = config.options.get("git-url").map(_.toString)

    (nameOpt, gitUrlOpt) match {
      case (Some(name), Some(gitUrl)) =>
        // This would call the TemplateManager.addGitTemplate method
        // For now, we'll just log the action
        val branch = config.options.get("branch").map(_.toString)
        info(
          s"Adding git template '$name' from URL '$gitUrl'${branch.map(b => s" (branch: $b)").getOrElse("")}"
        )
        true
      case (None, _) =>
        error("Missing required template name")
        false
      case (_, None) =>
        error("Missing required git URL")
        false
    }
  }

  /** Executes the 'template add-github' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateAddGitHub(config: Config): Boolean = {
    val nameOpt = config.templateName
    val ownerRepoOpt = config.options.get("owner/repo").map(_.toString)

    (nameOpt, ownerRepoOpt) match {
      case (Some(name), Some(ownerRepo)) =>
        // This would call the TemplateManager.addGitHubTemplate method
        // For now, we'll just log the action
        val ref = config.options.get("ref").map(_.toString)
        info(
          s"Adding GitHub template '$name' from '$ownerRepo'${ref.map(r => s" (ref: $r)").getOrElse("")}"
        )
        true
      case (None, _) =>
        error("Missing required template name")
        false
      case (_, None) =>
        error("Missing required owner/repo")
        false
    }
  }

  /** Executes the 'template remove' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateRemove(config: Config): Boolean = {
    config.templateName.fold {
      error("Missing required template name")
      false
    } { name =>
      // This would call the TemplateManager.removeTemplate method
      // For now, we'll just log the action
      info(s"Removing template '$name'")
      true
    }
  }

  /** Executes the 'template update' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateUpdate(config: Config): Boolean = {
    config.templateName.fold {
      error("Missing required template name")
      false
    } { name =>
      // This would call the TemplateManager.updateTemplate method
      // For now, we'll just log the action
      info(s"Updating template '$name'")
      true
    }
  }

  /** Executes the 'template update-all' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeTemplateUpdateAll(config: Config): Boolean = {
    // This would call the TemplateManager.updateAllTemplates method
    // For now, we'll just log the action
    info("Updating all templates")
    true
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
  private def loadProject(config: Config, filename: String): Overlord = {
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

  /** Ensures that standard resources are available.
    *
    * @param stdResourcePath
    *   The path to the standard resources
    * @param autoYes
    *   Whether to automatically answer yes to prompts
    * @return
    *   true if successful, false otherwise
    */
  private def ensureStdResources(
      stdResourcePath: Path,
      autoYes: Boolean
  ): Boolean = {
    if (!Files.exists(stdResourcePath) || !isValidGitRepo(stdResourcePath)) {
      val repoExists = Files.exists(stdResourcePath)
      val message = if (repoExists) {
        s"Standard resource folder '$stdResourcePath' exists but is not a valid Git repository."
      } else {
        s"Standard resource folder '$stdResourcePath' does not exist."
      }
      info(message)

      val shouldDownload = if (autoYes) {
        info(
          "Auto-downloading standard resource folder (-y/--yes specified)..."
        )
        true
      } else {
        info("Waiting for user input about downloading resources")
        print(
          "Would you like to download the standard catalog from Git? (y/n): "
        )
        val response = scala.io.StdIn.readLine()
        info(
          s"User response for download: ${if (response == "y") "yes" else "no"}"
        )
        response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
      }

      if (shouldDownload) {
        if (repoExists) {
          info("Removing invalid repository folder...")
          val removeCommand = s"rm -rf $stdResourcePath"
          val removeResult = removeCommand.!
          if (removeResult != 0) {
            error("Error: Failed to remove invalid repository folder.")
            return false
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
          return false
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
        warn("Download skipped. The standard catalog will not be available.")
        error(
          "Exiting as standard resources are required. Use --nostdresources to proceed without them."
        )
        return false
      }
    }

    true
  }

  /** Checks if a path is a valid git repository.
    *
    * @param path
    *   The path to check
    * @return
    *   true if the path is a valid git repository, false otherwise
    */
  private def isValidGitRepo(path: Path): Boolean = {
    val gitDirPath = path.resolve(".git")
    Files.exists(gitDirPath) && Files.isDirectory(gitDirPath)
  }

  /** Expands a path, replacing ~ with the user's home directory.
    *
    * @param path
    *   The path to expand
    * @return
    *   The expanded path
    */
  private def expandPath(path: String): String = {
    if (path.startsWith("~")) {
      path.replaceFirst("~", System.getProperty("user.home"))
    } else {
      path
    }
  }

  /** Executes the 'create default-templates' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeCreateDefaultTemplates(config: Config): Boolean = {
    info("Downloading standard templates...")

    val templates = TemplateManager.listAvailableTemplates()
    if (templates.nonEmpty) {
      info(
        "Some templates are already installed. The following templates are available:"
      )
      templates.foreach { template =>
        info(s"  $template")
      }

      val shouldDownload = if (config.yes) {
        info("Auto-downloading standard templates (-y/--yes specified)...")
        true
      } else {
        print(
          "Do you want to download or update the standard templates? (y/n): "
        )
        val response = scala.io.StdIn.readLine()
        response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
      }

      if (shouldDownload) {
        val result = TemplateManager.downloadStandardTemplates(config.yes)
        if (result) {
          info("Standard templates have been downloaded successfully.")
          val updatedTemplates = TemplateManager.listAvailableTemplates()
          info("Available templates:")
          updatedTemplates.foreach { template =>
            info(s"  $template")
          }
          return true
        } else {
          error("Failed to download some standard templates.")
          return false
        }
      } else {
        info("Template download skipped.")
        return true
      }
    } else {
      // No templates installed yet, proceed with download
      val result = TemplateManager.downloadStandardTemplates(config.yes)
      if (result) {
        info("Standard templates have been downloaded successfully.")
        val updatedTemplates = TemplateManager.listAvailableTemplates()
        info("Available templates:")
        updatedTemplates.foreach { template =>
          info(s"  $template")
        }
        return true
      } else {
        error("Failed to download standard templates.")
        return false
      }
    }
  }

  /** Executes the 'create gcc-toolchain' command.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  private def executeCreateGccToolchain(config: Config): Boolean = {
    val tripleOpt = config.options.get("triple").collect {
      case s: String if s.nonEmpty => s
    }
    val destinationOpt = config.options.get("destination").collect {
      case s: String if s.nonEmpty => s
    }.orElse(config.destination)

    val gccVersion = config.gccVersion.getOrElse("13.2.0")
    val binutilsVersion = config.options.getOrElse("binutils-version", "2.42").toString

    (tripleOpt, destinationOpt) match {
      case (Some(triple), Some(destination)) =>
        info(s"Building GCC toolchain for target triple '$triple'")
        info(s"Using GCC version: $gccVersion")
        info(s"Using binutils version: $binutilsVersion")
        info(s"Installing to: $destination")

        try {
          val expandedDestination = expandPath(destination)
          val destPath =
            Paths.get(expandedDestination).toAbsolutePath.normalize()

          // Ensure the destination directory exists
          if (!Files.exists(destPath)) {
            info(s"Creating destination directory: $expandedDestination")
            Files.createDirectories(destPath)
          }

          // Check if destination is writable
          if (!Files.isWritable(destPath)) {
            error(s"Destination directory $expandedDestination is not writable")
            return false
          }

          // Build the toolchain
          buildGccToolchain(
            triple,
            destPath.toString, // Use absolute path to avoid path duplication
            gccVersion,
            binutilsVersion
          )
        } catch {
          case e: Exception =>
            error(s"Failed to build GCC toolchain: ${e.getMessage}")
            e.printStackTrace() // Print stack trace for better debugging
            false
        }

      case (None, _) | (_, None) =>
        // Display ONLY the focused help text for the gcc-toolchain command
        println(HelpTextManager.getSubcommandHelp("create", "gcc-toolchain"))
        false
    }
  }

  /** Builds a GCC toolchain.
    *
    * @param triple
    *   The target triple (e.g., arm-none-eabi)
    * @param destination
    *   Where to install the toolchain
    * @param gccVersion
    *   The GCC version to use
    * @param binutilsVersion
    *   The binutils version to use
    * @return
    *   true if successful, false otherwis
    */
  private def buildGccToolchain(
      triple: String,
      destination: String,
      gccVersion: String,
      binutilsVersion: String
  ): Boolean = {
    info("Starting toolchain build process...")

    // Create a unique build ID for logging
    val buildId = java.util.UUID.randomUUID().toString.substring(0, 8)
    info(s"Build ID: $buildId")

    try {
      // Ensure destination directory exists and is writable
      val destPath = Paths.get(destination)
      if (!Files.exists(destPath)) {
        info(s"Creating destination directory: $destination")
        try {
          Files.createDirectories(destPath)
        } catch {
          case e: Exception =>
            error(s"Failed to create destination directory: ${e.getMessage}")
            return false
        }
      }

      // Check if destination is writable
      if (!Files.isWritable(destPath)) {
        error(s"Destination directory $destination is not writable")
        return false
      }

      // Generate CMake toolchain file
      val toolchainFile = Paths.get(destination, s"${triple}_toolchain.cmake")
      if (!generateCMakeToolchainFile(toolchainFile, triple, gccVersion)) {
        warn("Failed to generate CMake toolchain file, continuing with build")
      } else {
        info(s"Generated CMake toolchain file at: $toolchainFile")
      }

      // Run the build_baremetal_toolchain.sh script
      // Use absolute path to the script in the overlord project directory
      // Extract build_baremetal_toolchain.sh from resources to destination
      val scriptResource = getClass.getClassLoader.getResourceAsStream("build_baremetal_toolchain.sh")
      if (scriptResource == null) {
        error("Could not find build_baremetal_toolchain.sh in resources.")
        return false
      }
      val extractedScriptPath = Paths.get(destination, "build_baremetal_toolchain.sh")
      try {
        Files.copy(
          scriptResource,
          extractedScriptPath,
          java.nio.file.StandardCopyOption.REPLACE_EXISTING
        )
      } catch {
        case e: Exception =>
          error(s"Failed to extract build_baremetal_toolchain.sh: ${e.getMessage}")
          return false
      }
      // Set executable permissions
      try {
        import java.nio.file.attribute.PosixFilePermissions
        Files.setPosixFilePermissions(
          extractedScriptPath,
          PosixFilePermissions.fromString("rwxr-xr-x")
        )
      } catch {
        case e: Exception =>
          warn(s"Could not set executable permissions on script: ${e.getMessage}")
      }
      val scriptPath = extractedScriptPath.toAbsolutePath.toString

      // Run the script directly without changing directory
      val cmd = Seq("bash", scriptPath, triple, destination, "--gcc-version", gccVersion, "--binutils-version", binutilsVersion)
      info(s"Running: ${cmd.mkString(" ")}")

      val processLogger = ProcessLogger(
        (out: String) => info(out),
        (err: String) => warn(err)
      )

      // Execute in the current directory, not in the destination
      val exitCode = cmd.!(processLogger)

      if (exitCode != 0) {
        error(s"Toolchain build script failed with exit code $exitCode")
        return false
      }

      // Verify the toolchain was built successfully by checking for key binaries
      val gccBinary = Paths.get(destination, "bin", s"$triple-gcc")
      val gppBinary = Paths.get(destination, "bin", s"$triple-g++")

      if (!Files.exists(gccBinary) || !Files.exists(gppBinary)) {
        warn(
          "Toolchain build completed but key binaries are missing. Build may have failed."
        )
        return false
      }

      info(s"GCC toolchain built successfully and installed to $destination")
      info(s"CMake toolchain file available at: $toolchainFile")
      info(
        s"To use this toolchain with CMake, add: -DCMAKE_TOOLCHAIN_FILE=$toolchainFile"
      )
      true
    } catch {
      case e: Exception =>
        warn(s"Error building toolchain: ${e.getMessage}")
        e.printStackTrace() // Print stack trace for better debugging
        false
    }
  }

  /** Generates a CMake toolchain file based on the template.
    *
    * @param outputPath
    *   Path where the toolchain file should be written
    * @param triple
    *   The target triple
    * @param gccVersion
    *   The GCC version
    * @return
    *   true if successful, false otherwise
    */
  private def generateCMakeToolchainFile(
      outputPath: Path,
      triple: String,
      gccVersion: String
  ): Boolean = {
    var templateStream: java.io.InputStream = null

    try {
      // Try the correct template name first
      templateStream = getClass.getResourceAsStream("/toolchain_template.cmake")

      if (templateStream == null) {
        error("Could not find any toolchain template resource")
        return false
      }

      // Read template content
      val templateContent =
        scala.io.Source.fromInputStream(templateStream).mkString

      // Replace placeholders
      val gccFlags = triple match {
        case t if t.startsWith("arm") =>
          "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16"
        case t if t.startsWith("riscv32") => "-march=rv32imac -mabi=ilp32"
        case t if t.startsWith("riscv64") => "-march=rv64imac -mabi=lp64"
        case t if t.startsWith("x86_64")  => "-m64"
        case t if t.startsWith("i686")    => "-m32"
        case _                            => ""
      }

      val replacedContent = templateContent
        .replace("${triple}", triple)
        .replace("${version}", gccVersion)
        .replace("${GCC_FLAGS}", gccFlags)

      // Prepend set(COMPILER_PATH ...) line
      val parent = outputPath.getParent
      val absInstallPath = if (parent != null) parent.toAbsolutePath.toString else ""
      val compilerPathLine = s"""set(COMPILER_PATH "$absInstallPath")\n"""
      val content = compilerPathLine + replacedContent

      // Ensure parent directories exist
      if (parent != null && !Files.exists(parent)) {
        Files.createDirectories(parent)
      }

      // Write to output file
      Files.write(outputPath, content.getBytes)
      info(s"Successfully generated CMake toolchain file at: $outputPath")
      true
    } catch {
      case e: Exception =>
        error(s"Error generating CMake toolchain file: ${e.getMessage}")
        e.printStackTrace() // Print stack trace for better debugging
        false
    } finally {
      if (templateStream != null) {
        try {
          templateStream.close()
        } catch {
          case _: Exception => // Ignore close errors
        }
      }
    }
  }
}
