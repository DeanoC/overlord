import java.nio.file.{Files, Path}

import overlord.GameBuilder.{containerStack, pathStack}
import overlord.{Board, BoardCatalog, DefinitionCatalog, DefinitionCatalogs, Resources, _}

import scala.annotation.tailrec
import scala.collection.immutable.Map

object Main {
	private val usage =
		"""|Usage: overlord [create|report|svd] --options path filename
			 |     : create - generate a compile project with all sub parts at 'out'
			 |     : report - prints some info about over structure
			 |     : svd    - produce a CMSIS-SVD file on its own
			 |     : --out  - path where generated files should be placed
			 |     : --nostdresources - don't use any build in resource toml files
		   |     : --resources - TODO use specified path a root of resources tomls
		   |		 : filename should be a .over file with a chip layout"""
			.stripMargin

	def main(args: Array[String]): Unit = {
		if (args.length == 0) {
			println(usage);
			sys.exit(1)
		}
		type OptionMap = Map[Symbol, Any]

		@tailrec
		def nextOption(map: OptionMap, list: List[String]): OptionMap = {
			def isSwitch(s: String) = (s(0) == '-')

			list match {
				case Nil              => map
				case "create" :: tail =>
					nextOption(map ++ Map(Symbol("create") -> true), tail)
				case "report" :: tail =>
					nextOption(map ++ Map(Symbol("report") -> true), tail)
				case "svd" :: tail    =>
					nextOption(map ++ Map(Symbol("svd") -> true), tail)

				case ("--out" | "-o") :: value :: tail =>
					nextOption(map ++ Map(Symbol("out") -> value), tail)
				case "--nostdresources" :: tail      =>
					nextOption(map ++ Map(Symbol("nostdresources") -> true), tail)
				case "--resources" :: value :: tail  =>
					nextOption(map ++ Map(Symbol("resources") -> value), tail)

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
			println(usage);
			println("filename is required");
			sys.exit(1)
		}

		val filename = options(Symbol("infile")).asInstanceOf[String]
		if (!Files.exists(Path.of(filename))) {
			println(usage);
			println(s"${filename} does not exists");
			sys.exit(1)
		}

		pathStack.push(Path.of(new java.io.File(".").getCanonicalPath))
		val filePath = pathStack.top.resolve(filename)
		pathStack.push(filePath.getParent.toAbsolutePath)
		containerStack.push(None)

		val source       = scala.io.Source.fromFile(filePath.toFile)

		val stdResources = Resources()
		val resources    =
			if (!options.contains(Symbol("resources"))) None
			else Some(overlord.Resources(Path.of(options(Symbol("resources"))
				                                     .asInstanceOf[String])))

		val chipCatalogs =
			DefinitionCatalogs(
				{
					if (!options.contains(Symbol("nostdresources")))
						stdResources.loadCatalogs()
					else
						Map[String, DefinitionCatalog]()
				}.++ {
					resources match {
						case Some(r) => r.loadCatalogs()
						case None    => Map[String, DefinitionCatalog]()
					}
				}.toMap
				)

		val boardCatalog =
			BoardCatalog {
				{
					if (!options.contains(Symbol("nostdresources")))
						stdResources.loadBoards(chipCatalogs)
					else
						Map[String, Board]()
				}.++ {
					resources match {
						case Some(r) => r.loadBoards(chipCatalogs)
						case None    => Map[String, Board]()
					}
				}.toMap
			}

		val gameText = source.getLines().mkString("\n")
		val game     = Game.newGame(filename,
		                            gameText,
		                            chipCatalogs,
		                            boardCatalog) match {
			case Some(game) => game
			case None       =>
				println(s"Error parsing ${filename}")
				sys.exit()
		}
		val out      = Path.of(if (!options.contains(Symbol("out"))) "."
		                       else options(Symbol("out")).asInstanceOf[String]
		                       ).toAbsolutePath

		if (options.contains(Symbol("create"))) output.Project(game, out)
		else {
			if (options.contains(Symbol("report"))) output.Report(game, out)
			if (options.contains(Symbol("svd"))) output.Svd(game, out)
		}
	}
}