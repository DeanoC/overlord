package overlord

import toml._;
import java.nio.file.{Paths, Files}
import java.io.File

object Overlord {
  val usage = """|Usage: overlord filename
                 |     : filename should be a .over file with a chip layout""".stripMargin
  def main(args: Array[String]) : Unit = {
    if (args.length == 0) { println(usage); sys.exit(1) }
    val arglist = args.toList
    type OptionMap = Map[Symbol, Any]

    def nextOption(map : OptionMap, list: List[String]) : OptionMap = {
      def isSwitch(s : String) = (s(0) == '-')
      list match {
        case Nil => map
//        case "--max-size" :: value :: tail =>
//                               nextOption(map ++ Map(Symbol("maxsize") -> value.toInt), tail)
        case string :: opt2 :: tail if isSwitch(opt2) => 
                               nextOption(map ++ Map(Symbol("infile") -> string), list.tail)
        case string :: Nil =>  nextOption(map ++ Map(Symbol("infile") -> string), list.tail)
        case option :: tail => println("Unknown option "+option) 
                               sys.exit(1) 
      }
    }
    val options = nextOption(Map(),arglist)
    if( !options.contains(Symbol("infile")) ) { println(usage); println("filename is required"); sys.exit(1) }
    val filename = options(Symbol("infile")).asInstanceOf[String]
    if( !Files.exists(Paths.get(filename)) ) { println(usage); println(s"${filename} does not exists"); sys.exit(1) }

    val gameOverFile = Paths.get(filename).toAbsolutePath().toFile()
    val source = scala.io.Source.fromFile(gameOverFile)

    val motherOption = GameOver.loadGame(source.getLines mkString "\n")

    motherOption match {
      case Some(mother) => println( mother.board.name )
      case None => println("Error parsing ${filename} ")
    }

  }
}