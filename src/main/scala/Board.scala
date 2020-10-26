package overlord

import toml._;
import java.nio.file.{Paths, Files}
import java.io.File



abstract class Board( val name : String )
{ 
  val chips : Seq[Chip]
  val constraints : Array[BoardFeature]

  def SoCFromFile( name : String ) : Option[SystemOnChip] = {
    val path = Paths.get(s"src/main/resources/chips/${name}.chip")
    if( !Files.exists(path.toAbsolutePath()) ) return None

    val chipFile = path.toAbsolutePath().toFile()
    val source = scala.io.Source.fromFile(chipFile)

    SystemOnChip.parse(name, source.getLines mkString "\n")
  }
}

