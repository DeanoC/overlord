package overlord.cmdline

import toml._;
import java.nio.file.{ Files, Paths }
import java.io.File
import scala.collection.mutable.{ ArrayBuffer, HashMap }
import scala.collection.immutable.Map
import overlord.parser._

object Cmdline {
  val usage = """|Usage: overlord filename
                 |     : filename should be a .over file with a chip layout""".stripMargin

  def go(args: Array[String]): Unit = {
    if (args.length == 0) { println(usage); sys.exit(1) }
    val arglist = args.toList
    type OptionMap = Map[Symbol, Any]

    def nextOption(map: OptionMap, list: List[String]): OptionMap = {
      def isSwitch(s: String) = (s(0) == '-')
      
      list match {
        case Nil => map
        case "report" :: tail =>
          nextOption(map ++ Map(Symbol("report") -> true), tail)
        case "--nostdresources" :: tail =>
          nextOption(map ++ Map(Symbol("nostdresources") -> true), tail)
        case "--resources" :: value :: tail =>
          nextOption(map ++ Map(Symbol("resources") -> value), tail)

        case string :: opt2 :: tail if isSwitch(opt2) =>
          nextOption(map ++ Map(Symbol("infile") -> string), list.tail)
        case string :: Nil =>
          nextOption(map ++ Map(Symbol("infile") -> string), list.tail)
        case option :: tail =>
          println("Unknown option " + option)
          sys.exit(1)
      }
    }
    val options = nextOption(Map(), arglist)
    if (!options.contains(Symbol("infile"))) {
      println(usage); println("filename is required"); sys.exit(1)
    }
    val filename = options(Symbol("infile")).asInstanceOf[String]
    if (!Files.exists(Paths.get(filename))) {
      println(usage); println(s"${filename} does not exists"); sys.exit(1)
    }

    val gameOverFile = Paths.get(filename).toAbsolutePath().toFile()
    val source       = scala.io.Source.fromFile(gameOverFile)

    val stdResources = Resources()
    val resources =
      if (!options.contains(Symbol("resources"))) None
      else Some(Resources(options(Symbol("resources")).asInstanceOf[String]))

    val chipCatalogs =
      ChipCatalogs(
        {
          if (!options.contains(Symbol("nostdresources")))
            stdResources.loadCatalogs()
          else
            Map[String, ChipCatalog]()
        }.++ {
          resources match {
            case Some(r) => r.loadCatalogs()
            case None    => Map[String, ChipCatalog]()
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

    val gameText = source.getLines mkString "\n"
    val game = Game.newGame( gameText, chipCatalogs, boardCatalog) match {
      case Some(game) => game
      case None         => {
        println(s"Error parsing ${filename}"); 
        sys.exit(); null
      }
    }
    if (!options.contains(Symbol("report"))) {
      println(game)
    }
  }
}
