import gagameos._
import overlord._
import overlord.Project

import java.nio.file.{Files, Path, Paths}
import scala.annotation.tailrec
import sys.process._

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
		   |     : filename should be a .over file to use for the project"""
			.stripMargin

	def main(args: Array[String]): Unit = {

		if (args.isEmpty) {
			println(usage)
			println("No arguments provided.")
			sys.exit(1)
		}
		type OptionMap = Map[Symbol, Any]

		val options = MainUtils.nextOption(Map(), args.toList)
		if (!options.contains(Symbol("infile"))) {
			println(usage)
			println(s"Arguments passed: ${args.mkString(" ")}")
			println("Error: filename is required")
			sys.exit(1)
		}

		if (!options.contains(Symbol("board"))) {
			println(usage)
			println(s"Arguments passed: ${args.mkString(" ")}")
			println("Error: board name is required")
			sys.exit(1)
		}

		val filename = options(Symbol("infile")).asInstanceOf[String]
		if (!Files.exists(Paths.get(filename))) {
			println(usage)
			println(s"Error: $filename does not exist")
			sys.exit(1)
		}

		val filePath = Paths.get(filename).toAbsolutePath.normalize()
		val out = Paths.get(
			options.getOrElse(Symbol("out"), ".").asInstanceOf[String]
		).toAbsolutePath.normalize()
		Utils.ensureDirectories(out)

		Project.setupPaths(filePath.getParent, Resources.stdResourcePath(), Resources.stdResourcePath(), out)

		val stdResources = Resources(Resources.stdResourcePath())
		val resources = options.get(Symbol("resources")).map { path =>
			overlord.Resources(Paths.get(path.asInstanceOf[String]))
		}

		def isValidGitRepo(path: Path): Boolean = {
			val gitDirPath = path.resolve(".git")
			Files.exists(gitDirPath) && Files.isDirectory(gitDirPath)
		}

		if (!options.contains(Symbol("nostdresources")) && 
			(!Files.exists(Resources.stdResourcePath()) || !isValidGitRepo(Resources.stdResourcePath()))) {
			
			val repoExists = Files.exists(Resources.stdResourcePath())
			val message = if (repoExists) {
				s"Warning: Standard resource folder '${Resources.stdResourcePath()}' exists but is not a valid Git repository."
			} else {
				s"Warning: Standard resource folder '${Resources.stdResourcePath()}' does not exist."
			}
			println(message)
			
			val autoDownload = options.contains(Symbol("yes"))
			val shouldDownload = if (autoDownload) {
				println("Auto-downloading standard resource folder (-y/--yes specified)...")
				true
			} else {
				print("Would you like to download the standard catalog from Git? (y/n): ")
				val response = scala.io.StdIn.readLine().trim.toLowerCase
				response == "y" || response == "yes"
			}
			
			if (shouldDownload) {
				if (repoExists) {
					println("Removing invalid repository folder...")
					val removeCommand = s"rm -rf ${Resources.stdResourcePath()}"
					val removeResult = removeCommand.!
					if (removeResult != 0) {
						println("Error: Failed to remove invalid repository folder.")
						sys.exit(1)
					}
				}
				
				println("Cloning standard resource folder from Git repository...")
				val cloneCommand = s"git clone https://github.com/DeanoC/gagameos_stdcatalog.git ${Resources.stdResourcePath()}"
				val cloneResult = cloneCommand.!
				if (cloneResult != 0) {
					println("Error: Failed to clone the standard resource folder.")
					sys.exit(1)
				} else {
					println("Standard resource folder successfully downloaded.")
				}
			} else {
				println("Download skipped. The standard catalog will not be available.")
				println("Exiting as standard resources are required. Use --nostdresources to proceed without them.")
				sys.exit(1)
			}
		}

		val chipCatalog = new DefinitionCatalog
		if (!options.contains(Symbol("nostdresources"))) {
			chipCatalog.mergeNewDefinition(stdResources.loadCatalogs())
		}
		resources.foreach(r => chipCatalog.mergeNewDefinition(r.loadCatalogs()))

		val prefabCatalog = new PrefabCatalog
		if (!options.contains(Symbol("nostdprefabs"))) {
			prefabCatalog.prefabs ++= stdResources.loadPrefabs()
		}
		resources.foreach(r => prefabCatalog.prefabs ++= r.loadPrefabs())

		val gameName = filename.split('/').last.split('.').head
		val game = Project(gameName, options(Symbol("board")).asInstanceOf[String], filePath, chipCatalog, prefabCatalog) match {
			case Some(game) => game
			case None => return
		}

		if (options.contains(Symbol("create"))) {
			output.Project(game)
			println(s"** Project created at $out **")
		} else if (options.contains(Symbol("update"))) {
			val instance = options.get(Symbol("instance")).map(_.asInstanceOf[String])
			output.UpdateProject(game, instance)
		} else {
			if (options.contains(Symbol("report"))) output.Report(game)
			if (options.contains(Symbol("svd"))) output.Svd(game)
		}
	}
}

object MainUtils {
  type OptionMap = Map[Symbol, Any]

  @tailrec
  def nextOption(map: OptionMap, list: List[String]): OptionMap = {
    def isSwitch(s: String) = s.startsWith("-")

    list match {
      case Nil =>
        if (!map.contains(Symbol("infile")))
          throw new IllegalArgumentException("Missing required option: infile")
        if (!map.contains(Symbol("board")))
          throw new IllegalArgumentException("Missing required option: board")
        map
      case "create" :: tail =>
        nextOption(map + (Symbol("create") -> true), tail)
      case "update" :: tail =>
        nextOption(map + (Symbol("update") -> true), tail)
      case "report" :: tail =>
        nextOption(map + (Symbol("report") -> true), tail)
      case "svd" :: tail =>
        nextOption(map + (Symbol("svd") -> true), tail)

      case ("--out" | "-o") :: value :: tail =>
        nextOption(map + (Symbol("out") -> value), tail)
      case "--nostdresources" :: tail =>
        nextOption(map + (Symbol("nostdresources") -> true), tail)
      case "--nostdprefabs" :: tail =>
        nextOption(map + (Symbol("nostdprefabs") -> true), tail)
      case "--board" :: value :: tail =>
        nextOption(map + (Symbol("board") -> value), tail)
      case "--resources" :: value :: tail =>
        nextOption(map + (Symbol("resources") -> value), tail)
      case "--instance" :: value :: tail =>
        nextOption(map + (Symbol("instance") -> value), tail)
      case "-y" :: tail =>
        nextOption(map + (Symbol("yes") -> true), tail)
      case "--yes" :: tail =>
        nextOption(map + (Symbol("yes") -> true), tail)

      case string :: opt2 :: tail if isSwitch(opt2) =>
        nextOption(map + (Symbol("infile") -> string), list.tail)
      case string :: Nil =>
        nextOption(map + (Symbol("infile") -> string), list.tail)
      case option :: _ =>
        throw new IllegalArgumentException(s"Unknown option: $option")
    }
  }
}