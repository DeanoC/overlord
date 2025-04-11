import gagameos._
import overlord._
import overlord.Project

import java.nio.file.{Files, Path, Paths}
import scala.annotation.tailrec
import sys.process._
import scopt.OParser

object Main {
  private val usage =
    """|Usage: overlord [create|update|report|svd] --options path filename
       |     : create - generate a compile project with all sub parts at 'out'
       |     : update - update an existing project instance
       |     : report - prints some info about the overall structure
       |     : svd    - produce a CMSIS-SVD file on its own
       |     : --out  - path where generated files should be placed
       |     : --board - board definition to use
       |     : --nostdresources - don't use the standard catalog
       |     : --nostdprefabs - don't use the standard prefabs
       |     : --resources - use the specified path as the root of resources
       |     : --instance - specify the instance to update
       |     : -y, --yes - automatically agree (i.e. automatically download resource files without prompting)
       |     : filename should be a .over file to use for the project""".stripMargin

  case class Config(
      command: Option[String] = None,
      out: String = ".",
      board: Option[String] = None,
      nostdresources: Boolean = false,
      nostdprefabs: Boolean = false,
      resources: Option[String] = None,
      instance: Option[String] = None,
      yes: Boolean = false,
      infile: Option[String] = None
  )

  val parser: OParser[_, Config] = {
    val builder = OParser.builder[Config]
    import builder._
    OParser.sequence(
      programName("overlord"),
      head("overlord", "1.0"),
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
        .text("automatically agree (e.g., download resource files without prompting)"),
      arg[String]("<infile>")
        .optional()
        .action((x, c) => c.copy(infile = Some(x)))
        .text("filename should be a .over file to use for the project")
    )
  }

  def main(args: Array[String]): Unit = {
    OParser.parse(parser, args, Config()) match {
      case Some(config) =>
        val filename = config.infile.get
        val expandedFilename = if (filename.startsWith("~")) {
          filename.replaceFirst("~", System.getProperty("user.home"))
        } else {
          filename
        }
        if (!Files.exists(Paths.get(expandedFilename))) {
          println(usage)
          println(s"Error: $expandedFilename does not exist")
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

        Project.setupPaths(
          filePath.getParent,
          Resources.stdResourcePath(),
          Resources.stdResourcePath(),
          out
        )

        val stdResourcePath = config.resources
          .map(path => Paths.get(path).toAbsolutePath.normalize())
          .getOrElse(Resources.stdResourcePath())
        Resources.setStdResourcePath(stdResourcePath)

        val resources = config.resources.map { path =>
          overlord.Resources(Paths.get(path))
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
            s"Warning: Standard resource folder '$stdResourcePath' exists but is not a valid Git repository."
          } else {
            s"Warning: Standard resource folder '$stdResourcePath' does not exist."
          }
          println(message)

          val autoDownload = config.yes
          val shouldDownload = if (autoDownload) {
            println(
              "Auto-downloading standard resource folder (-y/--yes specified)..."
            )
            true
          } else {
            print(
              "Would you like to download the standard catalog from Git? (y/n): "
            )
            val response = scala.io.StdIn.readLine()
            response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
          }

          if (shouldDownload) {
            if (repoExists) {
              println("Removing invalid repository folder...")
              val removeCommand = s"rm -rf $stdResourcePath"
              val removeResult = removeCommand.!
              if (removeResult != 0) {
                println("Error: Failed to remove invalid repository folder.")
                sys.exit(1)
              }
            }

            // Ensure all parent directories for stdResourcePath exist
            val parentDir = stdResourcePath.getParent
            if (!Files.exists(parentDir)) {
              Files.createDirectories(parentDir)
            }

            println("Cloning standard resource folder from Git repository...")
            val cloneCommand =
              s"git clone https://github.com/DeanoC/gagameos_stdcatalog.git $stdResourcePath"
            val cloneResult = cloneCommand.!
            if (cloneResult != 0) {
              println("Error: Failed to clone the standard resource folder.")
              sys.exit(1)
            } else {
              println("Standard resource folder successfully downloaded.")
              
              // Initialize and update git submodules
              println("Initializing git submodules...")
              val initCommand = Process(Seq("git", "submodule", "init"), new java.io.File(stdResourcePath.toString))
              val initResult = initCommand.!
              if (initResult != 0) {
                println("Warning: Failed to initialize git submodules.")
              } else {
                println("Updating git submodules...")
                val updateCommand = Process(Seq("git", "submodule", "update", "--recursive"), new java.io.File(stdResourcePath.toString))
                val updateResult = updateCommand.!
                if (updateResult != 0) {
                  println("Warning: Failed to update git submodules.")
                } else {
                  println("Git submodules successfully initialized and updated.")
                }
              }
            }
          } else {
            println("Download skipped. The standard catalog will not be available.")
            println(
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
            println(s"** Project created at $out **")
          case Some("update") =>
            output.UpdateProject(game, config.instance)
          case _ =>
            if (config.command.contains("report")) output.Report(game)
            if (config.command.contains("svd")) output.Svd(game)
        }

      case None =>
        println(usage)
        sys.exit(1)
    }
  }
}