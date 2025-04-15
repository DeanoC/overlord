package com.deanoc.overlord.cli

import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.DefinitionCatalog
import com.deanoc.overlord.PrefabCatalog
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

  /** Executes a command based on the configuration.
    *
    * @param config
    *   The parsed configuration
    * @return
    *   true if successful, false otherwise
    */
  def execute(config: Config): Boolean = {
    (config.command, config.subCommand) match {
      // CREATE commands
      case (Some("create"), Some("project")) =>
        executeCreateProject(config)

      case (Some("create"), Some("default-templates")) =>
        executeCreateDefaultTemplates(config)

      case (Some("create"), Some("gcc-toolchain")) =>
        executeCreateGccToolchain(config)

      // GENERATE commands
      case (Some("generate"), Some("test")) =>
        executeGenerateTest(config)

      case (Some("generate"), Some("report")) =>
        executeGenerateReport(config)

      case (Some("generate"), Some("svd")) =>
        executeGenerateSvd(config)

      // CLEAN commands
      case (Some("clean"), Some("test")) =>
        executeCleanTest(config)

      // UPDATE commands
      case (Some("update"), Some("project")) =>
        executeUpdateProject(config)

      case (Some("update"), Some("catalog")) =>
        executeUpdateCatalog(config)

      // TEMPLATE commands
      case (Some("template"), Some("list")) =>
        executeTemplateList(config)

      case (Some("template"), Some("add")) =>
        executeTemplateAdd(config)

      case (Some("template"), Some("add-git")) =>
        executeTemplateAddGit(config)

      case (Some("template"), Some("add-github")) =>
        executeTemplateAddGitHub(config)

      case (Some("template"), Some("remove")) =>
        executeTemplateRemove(config)

      case (Some("template"), Some("update")) =>
        executeTemplateUpdate(config)

      case (Some("template"), Some("update-all")) =>
        executeTemplateUpdateAll(config)

      // Unknown command combination
      case _ =>
        error("Unknown command or missing subcommand")
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
    config.infile match {
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
    config.infile match {
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
    config.infile match {
      case Some(filename) =>
        try {
          val game = loadProject(config, filename)
          if (game != null) {
            output.UpdateProject(game, config.instance)
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
    val gitUrlOpt = config.gitUrl

    (nameOpt, gitUrlOpt) match {
      case (Some(name), Some(gitUrl)) =>
        // This would call the TemplateManager.addGitTemplate method
        // For now, we'll just log the action
        info(
          s"Adding git template '$name' from URL '$gitUrl'${config.branch
              .map(b => s" (branch: $b)")
              .getOrElse("")}"
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
    val ownerRepoOpt = config.ownerRepo

    (nameOpt, ownerRepoOpt) match {
      case (Some(name), Some(ownerRepo)) =>
        // This would call the TemplateManager.addGitHubTemplate method
        // For now, we'll just log the action
        info(
          s"Adding GitHub template '$name' from '$ownerRepo'${config.ref
              .map(r => s" (ref: $r)")
              .getOrElse("")}"
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

    System.setProperty("user.dir", parentDir.toString)

    // Remove reference to config.out
    // Use the project file's directory
    Utils.ensureDirectories(parentDir)

    Overlord.setupPaths(filePath)

    val gameName = filename.split('/').last.split('.').head
    val board = config.board.getOrElse("unknown")

    Overlord(gameName, board, filePath)
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
    }

    val gccVersion = config.options.getOrElse("gcc-version", "13.2.0").toString
    val binutilsVersion =
      config.options.getOrElse("binutils-version", "2.42").toString

    (tripleOpt, destinationOpt) match {
      case (Some(triple), Some(destination)) =>
        info(s"Building GCC toolchain for target triple '$triple'")
        info(s"Using GCC version: $gccVersion")
        info(s"Using binutils version: $binutilsVersion")
        info(s"Installing to: $destination")

        try {
          val expandedDestination = expandPath(destination)
          val destPath = Paths.get(expandedDestination)

          // Ensure the destination directory exists
          if (!Files.exists(destPath)) {
            info(s"Creating destination directory: $expandedDestination")
            Files.createDirectories(destPath)
          }

          // Build the toolchain
          buildGccToolchain(
            triple,
            expandedDestination,
            gccVersion,
            binutilsVersion
          )
        } catch {
          case e: Exception =>
            error(s"Failed to build GCC toolchain: ${e.getMessage}")
            false
        }

      case (None, _) =>
        error("Missing required argument: triple")
        false
      case (_, None) =>
        error("Missing required argument: destination")
        false
    }
  }

  /** Builds a GCC toolchain using the create_gcc.sh script.
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
    *   true if successful, false otherwise
    */
  private def buildGccToolchain(
      triple: String,
      destination: String,
      gccVersion: String,
      binutilsVersion: String
  ): Boolean = {
    info("Starting toolchain build process...")

    try {
      // Create a workspace directory for downloads
      val workspaceDir = Files.createTempDirectory("gcc-workspace-")
      info(s"Created temporary workspace directory: $workspaceDir")

      // Generate CMake toolchain file
      val toolchainFile = Paths.get(destination, s"${triple}_toolchain.cmake")
      if (!generateCMakeToolchainFile(toolchainFile, triple, gccVersion)) {
        warn("Failed to generate CMake toolchain file, continuing with build")
      } else {
        info(s"Generated CMake toolchain file at: $toolchainFile")
      }

      // Copy the script to the destination folder
      val scriptResource = getClass.getResourceAsStream("/create_gcc.sh")
      if (scriptResource == null) {
        error("Could not find create_gcc.sh resource")
        return false
      }

      val scriptDestPath = Paths.get(destination, "create_gcc.sh")
      info(s"Copying build script to: $scriptDestPath")

      // Make sure we don't have an existing file
      try {
        Files.deleteIfExists(scriptDestPath)
      } catch {
        case _: Exception => // Ignore errors when trying to delete
      }

      // Copy the resource and close the stream properly
      try {
        val destStream = Files.newOutputStream(scriptDestPath)
        val buffer = new Array[Byte](4096)
        var bytesRead = 0
        while ({ bytesRead = scriptResource.read(buffer); bytesRead != -1 }) {
          destStream.write(buffer, 0, bytesRead)
        }
        destStream.close()
        scriptResource.close()
      } catch {
        case e: Exception =>
          error(s"Failed to copy script: ${e.getMessage}")
          return false
      }

      // Make the script executable
      val execCmd = s"chmod +x ${scriptDestPath.toAbsolutePath}"
      if (execCmd.! != 0) {
        error("Failed to make script executable")
        return false
      }

      // Define environment variables
      val env = Seq(
        "TARGET" -> triple,
        "GCC_VERSION" -> gccVersion,
        "BINUTILS_VERSION" -> binutilsVersion,
        "INSTALL_DIR" -> destination
      )

      // Execute the script with absolute path
      info("Executing GCC build script...")
      val processBuilder = Process(
        Seq("bash", scriptDestPath.toAbsolutePath.toString),
        new java.io.File(destination),
        env: _*
      )

      val processLogger = ProcessLogger(
        line => info(s"[build] $line"),
        line => error(s"[build-error] $line")
      )

      val exitCode = processBuilder ! processLogger
      if (exitCode != 0) {
        error(s"Build script exited with code $exitCode")
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
        error(s"Error building toolchain: ${e.getMessage}")
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
    try {
      val templateStream =
        getClass.getResourceAsStream("/toolchain_teamplate.cmake")
      if (templateStream == null) {
        error("Could not find toolchain template resource")
        return false
      }

      // Read template content
      val templateContent =
        scala.io.Source.fromInputStream(templateStream).mkString
      templateStream.close()

      // Replace placeholders
      val gccFlags = triple match {
        case t if t.startsWith("arm") =>
          "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16"
        case t if t.startsWith("riscv") => "-march=rv32imac -mabi=ilp32"
        case _                          => ""
      }

      val content = templateContent
        .replace("${triple}", triple)
        .replace("${version}", gccVersion)
        .replace("${GCC_FLAGS}", gccFlags)

      // Write to output file
      Files.write(outputPath, content.getBytes)
      true
    } catch {
      case e: Exception =>
        error(s"Error generating CMake toolchain file: ${e.getMessage}")
        false
    }
  }
}
