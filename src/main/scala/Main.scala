import gagameos._
import overlord._

import java.nio.file.{Files, Path, Paths}
import scala.annotation.tailrec
import sys.process._

object Main {
	private val usage =
		"""|Usage: overlord [create|report|svd] --options path filename
		   |     : create - generate a compile project with all sub parts at 'out'
		   |     : report - prints some info about the overall structure
		   |     : svd    - produce a CMSIS-SVD file on its own
		   |     : --out  - path where generated files should be placed
		   |     : --board - board definition to use
		   |     : --nostdresources - don't use the standard catalog
		   |     : --resources - use the specified path as the root of resources
		   |     : filename should be a .over file to use for the project"""
			.stripMargin

	def main(args: Array[String]): Unit = {

		if (args.isEmpty) {
			println(usage)
			println("No arguments provided.")
			sys.exit(1)
		}
		type OptionMap = Map[Symbol, Any]

		@tailrec
		def nextOption(map: OptionMap, list: List[String]): OptionMap = {
			def isSwitch(s: String) = s.startsWith("-")

			list match {
				case Nil              => map
				case "create" :: tail =>
					nextOption(map + (Symbol("create") -> true), tail)
				case "update" :: tail =>
					nextOption(map + (Symbol("update") -> true), tail)
				case "report" :: tail =>
					nextOption(map + (Symbol("report") -> true), tail)
				case "svd" :: tail    =>
					nextOption(map + (Symbol("svd") -> true), tail)

				case ("--out" | "-o") :: value :: tail =>
					nextOption(map + (Symbol("out") -> value), tail)
				case "--nostdresources" :: tail        =>
					nextOption(map + (Symbol("nostdresources") -> true), tail)
				case "--nostdprefabs" :: tail          =>
					nextOption(map + (Symbol("nostdprefabs") -> true), tail)
				case "--board" :: value :: tail        =>
					nextOption(map + (Symbol("board") -> value), tail)
				case "--resources" :: value :: tail    =>
					nextOption(map + (Symbol("resources") -> value), tail)
				case "--instance" :: value :: tail     =>
					nextOption(map + (Symbol("instance") -> value), tail)

				case string :: opt2 :: tail if isSwitch(opt2) =>
					nextOption(map + (Symbol("infile") -> string), list.tail)
				case string :: Nil                            =>
					nextOption(map + (Symbol("infile") -> string), list.tail)
				case option :: _                              =>
					println(s"Unknown option: $option")
					println(usage)
					println(s"Arguments passed: ${args.mkString(" ")}")
					sys.exit(1)
			}
		}

		val options = nextOption(Map(), args.toList)
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

		Game.setupPaths(filePath.getParent, Resources.stdResourcePath(), Resources.stdResourcePath(), out)

		val stdResources = Resources(Resources.stdResourcePath())
		val resources = options.get(Symbol("resources")).map { path =>
			overlord.Resources(Paths.get(path.asInstanceOf[String]))
		}

		if (!options.contains(Symbol("nostdresources")) && !Files.exists(Resources.stdResourcePath())) {
			println(s"Warning: Standard resource folder '${Resources.stdResourcePath()}' does not exist.")
			println("Cloning standard resource folder from Git repository...")
			val cloneCommand = s"git clone https://github.com/DeanoC/gagameos_stdcatalog.git ${Resources.stdResourcePath()}"
			val cloneResult = cloneCommand.!
			if (cloneResult != 0) {
				println("Error: Failed to clone the standard resource folder.")
				sys.exit(1)
			}
		}

		if (!options.contains(Symbol("nostdresources")) && !Files.exists(Resources.stdResourcePath())) {
			println(s"Warning: Standard resource folder '${Resources.stdResourcePath()}' does not exist.")
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

		val game = Game(gameName, options(Symbol("board")).asInstanceOf[String], filePath, chipCatalog, prefabCatalog) match {
			case Some(game) => game
			case None       =>
				println(s"Error parsing $filename")
				sys.exit(1)
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