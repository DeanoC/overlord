package overlord
import toml._;
import scala.language.postfixOps

object GameOver
{
  def loadGame( game : String ) : Option[Mother] = {
    if(game.isEmpty) return None

    val parsed = {
      val tparsed = toml.Toml.parse(game)
      if(tparsed.isLeft) return None      
      tparsed.right.get
    }

    val board = parsed.values("board").asInstanceOf[toml.Value.Str].value match {
      case "Myir-FZ3" => {
        boards.MyirFZ3()
      }
      case "Pynq-Z2" => {
        boards.PynqZ2()
      }

      case _ => return None
    }

    // TODO add non board chipLibraries's from game.over
    val chipLibraries = Array[Chip]()

    val mother = new Mother(board, chipLibraries)

    val layout = parsed.values("layout").asInstanceOf[toml.Value.Arr].values;
    for( chip <- layout) {
      val chipTable = chip.asInstanceOf[toml.Value.Tbl]
      val chipName = chipTable.values("name").asInstanceOf[toml.Value.Str].value
      
      if(mother.hasChip(chipName) ) {
        if( mother.otherChips(chipName).isInstanceOf[SystemOnChip] ) {
          println(f"${chipName} SoC added with")
          val soc = mother.otherChips(chipName).asInstanceOf[SystemOnChip]
          soc.chips.foreach( c => println(s"\t${c.name}"))
        } else {
          println(f"${chipName} added")
        }
      } else {
        println(f"${chipName} not found in any chip libraries")
      }

    }


    Some(mother)
  }
}