package com.deanoc.overlord.cli.commands.create

import com.deanoc.overlord.cli.{CliConfig, HelpTextManager}
import com.deanoc.overlord.cli.commands.CommandHandler

import java.nio.file.{Files, Path, Paths}
import scala.sys.process._

object GccToolchainCreateHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    val tripleOpt = config.options.get("triple").collect {
      case s: String if s.nonEmpty => s
    }
    val destinationOpt = config.options
      .get("destination")
      .collect {
        case s: String if s.nonEmpty => s
      }
      .orElse(config.destination)

    val gccVersion = config.gccVersion.getOrElse("13.2.0")
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
      val scriptResource = getClass.getClassLoader.getResourceAsStream(
        "build_baremetal_toolchain.sh"
      )
      if (scriptResource == null) {
        error("Could not find build_baremetal_toolchain.sh in resources.")
        return false
      }
      val extractedScriptPath =
        Paths.get(destination, "build_baremetal_toolchain.sh")
      try {
        Files.copy(
          scriptResource,
          extractedScriptPath,
          java.nio.file.StandardCopyOption.REPLACE_EXISTING
        )
      } catch {
        case e: Exception =>
          error(
            s"Failed to extract build_baremetal_toolchain.sh: ${e.getMessage}"
          )
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
          warn(
            s"Could not set executable permissions on script: ${e.getMessage}"
          )
      }
      val scriptPath = extractedScriptPath.toAbsolutePath.toString

      // Run the script directly without changing directory
      val cmd = Seq(
        "bash",
        scriptPath,
        triple,
        destination,
        "--gcc-version",
        gccVersion,
        "--binutils-version",
        binutilsVersion
      )
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
      val absInstallPath =
        if (parent != null) parent.toAbsolutePath.toString else ""
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
