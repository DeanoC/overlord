import ikuy_utils._
import overlord._

import java.nio.file.{Files, Path}
import scala.annotation.tailrec

object Main {
	private val usage =
		"""|Usage: overlord [create|report|svd] --options path filename
		   |     : create - generate a compile project with all sub parts at 'out'
		   |     : report - prints some info about over structure
		   |     : svd    - produce a CMSIS-SVD file on its own
		   |     : --out  - path where generated files should be placed
		   |     : --board - board defination to use
		   |     : --nostdresources - don't use any the std catalog
		   |     : --resources - TODO use specified path a root of resources tomls
		   |		 : filename should be a .over file to use for the project"""
			.stripMargin

	def main(args: Array[String]): Unit = {
		if (args.length == 0) {
			println(usage)
			sys.exit(1)
		}
		type OptionMap = Map[Symbol, Any]

		@tailrec
		def nextOption(map: OptionMap, list: List[String]): OptionMap = {
			def isSwitch(s: String) = s(0) == '-'

			list match {
				case Nil              => map
				case "create" :: tail =>
					nextOption(map ++ Map(Symbol("create") -> true), tail)
				case "update" :: tail =>
					nextOption(map ++ Map(Symbol("update") -> true), tail)
				case "report" :: tail =>
					nextOption(map ++ Map(Symbol("report") -> true), tail)
				case "svd" :: tail    =>
					nextOption(map ++ Map(Symbol("svd") -> true), tail)

				case ("--out" | "-o") :: value :: tail =>
					nextOption(map ++ Map(Symbol("out") -> value), tail)
				case "--nostdresources" :: tail        =>
					nextOption(map ++ Map(Symbol("nostdresources") -> true), tail)
				case "--nostdprefabs" :: tail          =>
					nextOption(map ++ Map(Symbol("nostdresources") -> true), tail)
				case "--board" :: value :: tail        =>
					nextOption(map ++ Map(Symbol("board") -> value), tail)
				case "--resources" :: value :: tail    =>
					nextOption(map ++ Map(Symbol("resources") -> value), tail)
				case "--instance" :: value :: tail     =>
					nextOption(map ++ Map(Symbol("instance") -> value), tail)

				case string :: opt2 :: tail if isSwitch(opt2) =>
					nextOption(map ++ Map(Symbol("infile") -> string), list.tail)
				case string :: Nil                            =>
					nextOption(map ++ Map(Symbol("infile") -> string), list.tail)
				case option :: tail                           =>
					println("Unknown option " + option)
					sys.exit(1)
			}
		}

		val options = nextOption(Map(), args.toList)
		if (!options.contains(Symbol("infile"))) {
			println(usage)
			println("filename is required")
			sys.exit(1)
		}

		if (!options.contains(Symbol("board"))) {
			println(usage)
			println("board name is required")
			println(options)
			sys.exit(1)
		}

		val board = options(Symbol("board")).asInstanceOf[String]

		val filename = options(Symbol("infile")).asInstanceOf[String]
		if (!Files.exists(Path.of(filename))) {
			println(usage)
			println(s"$filename does not exists")
			sys.exit(1)
		}
		val filePath = Path.of(filename)
		val out      = Path.of(
			if (!options.contains(Symbol("out"))) "."
			else options(Symbol("out")).asInstanceOf[String]
			).toAbsolutePath
		Utils.ensureDirectories(out)

		Game.setupPaths(filePath.getParent, Resources.stdResourcePath(), Resources.stdResourcePath(), out)

		val stdResources = Resources(Resources.stdResourcePath())

		val resources =
			if (!options.contains(Symbol("resources"))) None
			else Some(overlord.Resources(Path.of(options(Symbol("resources"))
				                                     .asInstanceOf[String])))


		val chipCatalog = new DefinitionCatalog

		if (!options.contains(Symbol("nostdresources")))
			chipCatalog.mergeNewDefinition(stdResources.loadCatalogs())
		if (resources.isDefined)
			chipCatalog.mergeNewDefinition(resources.get.loadCatalogs())

		val prefabCatalog = new PrefabCatalog
		if (!options.contains(Symbol("nostdprefabs")))
			prefabCatalog.prefabs ++= stdResources.loadPrefabs()
		if (resources.isDefined)
			prefabCatalog.prefabs ++= resources.get.loadPrefabs()

		val gameName = filename.split('/').last.split('.').head

		Game.setupPaths(filePath.getParent, filePath.getParent, filePath.getParent, out)

		val game = Game(gameName, board, filePath, chipCatalog, prefabCatalog) match {
			case Some(game) => game
			case None       =>
				println(s"Error parsing $filename")
				sys.exit()
		}

		if (options.contains(Symbol("create"))) output.Project(game)
		if (options.contains(Symbol("update"))) {
			val instance = if (!options.contains(Symbol("instance"))) None
			else Some(options(Symbol("instance")).asInstanceOf[String])
			output.UpdateProject(game, instance)
		}
		else {
			if (options.contains(Symbol("report"))) output.Report(game)
			if (options.contains(Symbol("svd"))) output.Svd(game)
		}
	}
}