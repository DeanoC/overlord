package overlord.parser

import scala.collection.immutable.Map
import scala.collection.mutable.{ ArrayBuffer, HashMap }
import java.nio.file.{ Files, Paths }
import java.io.File
import toml._

sealed trait BoardType {
  val defaultConstraints: Map[String, toml.Value]
}

case class XilinxBoard() extends BoardType {
  override val defaultConstraints = Map[String, toml.Value](
    ("pullup" -> toml.Value.Bool(false)),
    ("slew" -> toml.Value.Str("Slow")),
    ("drive" -> toml.Value.Num(8)),
    ("direction" -> toml.Value.Str("None")),
    ("standard" -> toml.Value.Str("LVCMOS33"))
  )
}
case class AlteraBoard() extends BoardType {
  override val defaultConstraints = Map[String, toml.Value]()
}

case class LatticeBoard() extends BoardType {
  override val defaultConstraints = Map[String, toml.Value]()
}

case class BoardCatalog(val catalog: Map[String, Board]) {
  def FindBoard(boardName: String): Option[Board] =
    if (catalog.contains(boardName)) Some(catalog(boardName))
    else {
      println(s"${boardName} not found in the board catalog"); return None
    }
}

case class Board(
  val boardType: BoardType,
  val name: String,
  instances: List[toml.Value],
  connections: Map[String, ConnectionDef],
  constraints: Map[String, Constraint]
)

object Board {

  def FromFile(
    spath: String,
    name: String,
    catalogs: ChipCatalogs
  ): Option[Board] = {
    println(s"Reading $name board")

    val path = Paths.get(s"$spath/${name}.toml")
    if (!Files.exists(path.toAbsolutePath())) {
      println(s"${name} board at ${path} not found"); return None
    }

    val chipFile = path.toAbsolutePath().toFile()
    val source = scala.io.Source.fromFile(chipFile)

    parse(name, source.getLines mkString "\n", catalogs)
  }

  private def parse(
    name: String,
    source: String,
    catalogs: ChipCatalogs
  ): Option[Board] = {
    import toml.Value._

    val parsed = {
      val tparsed = toml.Toml.parse(source)
      if (tparsed.isLeft) {
        println(
          s"$name board has failed to parse with error ${tparsed.left.get}"
        ); return None
      }
      tparsed.right.get.values
    }
    // what type of board?
    val boardType =
      parsed("type").asInstanceOf[Value.Str].value match {
        case "Xilinx"  => XilinxBoard()
        case "Altera"  => AlteraBoard()
        case "Lattice" => LatticeBoard()
        case _         => println(s"$name board has a unknown type"); return None
      }

    // parse defaults
    var defaultConstraints =
      HashMap[String, Value](boardType.defaultConstraints.toSeq: _*)

    if (parsed.contains("constraints_default")) {
      val defaultsTable =
        parsed("constraints_default").asInstanceOf[Value.Tbl].values
      for {
        (defaultName, defaultValue) <- defaultConstraints
        if (defaultsTable.contains(defaultName))
      } defaultConstraints(defaultName) = defaultsTable(defaultName)
    }
    if (!parsed.contains("type")) {
      println(s"${name} board requires a type value"); return None
    }
    var boardChips = HashMap[String, InstanceDef]()
    var boardConnections = HashMap[String, ConnectionDef]()

    // search catalogs for chips on the board
    val boardInstances= if (parsed.contains("instance")) {
      parsed("instance").asInstanceOf[Value.Arr].values
    } else List[toml.Value]()
/*
    // search catalogs for rams on the board
    if (parsed.contains("rams")) {
      val tinstances =
        parsed("rams").asInstanceOf[Value.Arr].values
      for (instance <- tinstances) {
        val table = instance.asInstanceOf[Value.Tbl].values
        val chipType = table("type").asInstanceOf[Value.Str].value
        val count =
          if (table.contains("count"))
            table("count").asInstanceOf[Value.Num].value.toInt
          else 1

        val name =
          if (!table.contains("name")) chipType
          else table("name").asInstanceOf[Value.Str].value

        val attribs: Map[String, Value] = table.filter(_._1 match {
          case "type" | "name" | "type" | "count" => false
          case _                                  => true
        })

        var found = catalogs.FindChip(chipType.split('.')(1))
        boardConnections ++= (found match {
          case Some(ramType) =>
            if (ramType.chipType.split('.')(0) == "ram")
              if (count == 1)
                Seq((name -> RamInstanceDef(name, ramType, attribs)))
              else
                for (index <- 0 until count)
                  yield (s"$name$index" -> RamInstanceDef(
                        s"$name$index",
                        ramType,
                        attribs
                      ))
            else {
              println(s"${chipType} isn't a ram chip")
              return None
            }
          case None =>
            println(s"${chipType} not found in any chip catalogs")
            return None
        })
      }
    }
*/
    var boardConstraints = HashMap[String, Constraint]()

    if (parsed.contains("constraint")) {
      val tconstraints =
        parsed("constraint").asInstanceOf[Value.Arr].values
      for (constraint <- tconstraints) {
        val table = constraint.asInstanceOf[Value.Tbl].values
        boardConstraints ++= Constraint.parse(table, defaultConstraints.toMap)
      }
    }

    Some(
      Board(
        boardType,
        name,
        boardInstances,
        boardConnections.toMap,
        boardConstraints.toMap
      )
    )
  }
}
