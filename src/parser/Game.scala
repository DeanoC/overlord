package overlord.parser

import scala.collection.mutable.{ ArrayBuffer, HashMap }
import scala.language.postfixOps
import java.nio.file.{ Files, Paths }
import java.io.File
import toml._

private case class GameGatherer(boardName: String, board: Board) {
  val connections = HashMap[String, ConnectionDef](board.connections.toSeq: _*)
  val interconnects = ArrayBuffer[InterconnectDef]()
  val instances = ArrayBuffer[InstanceDef]()
}

case class Game(
  connections: List[ConnectionDef],
  interconnects: List[InterconnectDef],
  instances: List[InstanceDef]
)

object Game {
  def newGame(
    gameText: String,
    catalogs: ChipCatalogs,
    boards: BoardCatalog
  ): Option[Game] = {
    if (gameText.isEmpty) return None

    val parsed = {
      val tparsed = toml.Toml.parse(gameText)
      if (tparsed.isLeft) {
        println(
          s"game.over has failed to parse with error ${tparsed.left.get}"
        ); return None
      }
      tparsed.right.get
    }

    if (!parsed.values.contains("board")) {
      println(s"game.over requires a board value")
      return None
    }

    // fix name (remove '-')
    val boardName = parsed
      .values("board")
      .asInstanceOf[toml.Value.Str]
      .value
      .filterNot(c => c == '-')
    val board = (boards.FindBoard(boardName) match {
      case Some(b) => b
      case None    => return None
    })

    var gatherer = GameGatherer(boardName, board)
    // search catalogs for chips on the board
    parseInstances(board.instances, gatherer, catalogs)

    // we "include" outself to use the same code for gather catalogs etc.
    includeOver(gatherer, gameText, catalogs)

    Some(
      Game(
        gatherer.connections.map(_._2).toList,
        gatherer.interconnects.toList,
        gatherer.instances.toList
      )
    )
  }

  // TODO scala generic code for these
  private def tableIntGetOrElse(
    table: Map[String, toml.Value],
    key: String,
    default: Int
  ) =
    if (table.contains(key))
      table(key).asInstanceOf[toml.Value.Num].value.toInt
    else default
  private def tableStringGetOrElse(
    table: Map[String, toml.Value],
    key: String,
    default: String
  ) =
    if (table.contains(key))
      table(key).asInstanceOf[toml.Value.Str].value
    else default

  private def parseInstances(
    tinstances: List[toml.Value],
    gatherer: GameGatherer,
    catalogs: ChipCatalogs
  ) {
    import toml.Value

    for (instance <- tinstances) {
      val table = instance.asInstanceOf[Value.Tbl].values
      val chipType = table("type").asInstanceOf[Value.Str].value

      val count = tableIntGetOrElse(table, "count", 1)
      val name = tableStringGetOrElse(table, "name", chipType)

      val attribs: Map[String, Value] =
        table.filter(_._1 match {
          case "type" | "name" | "type" | "count" => false
          case _                                  => true
        })

      gatherer.instances ++= (catalogs.FindChip(chipType) match {
        case Some(c) =>
          val names =
            if (count == 1) Seq(name)
            else for (index <- 0 until count) yield s"$name.$index"

          for (name <- names) yield chipType.split('.')(0) match {
            case "ram" => InstanceDef(name, c, attribs)
            case "cpu" => InstanceDef(name, c, attribs)
            case "NxMinterconnect" => InstanceDef(name, c, attribs)
            case "storage" => InstanceDef(name, c, attribs)
            case "soc" => InstanceDef(name, c, attribs)
            case _ =>
              println(s"$chipType has unknown chip class")
              InstanceDef(name, c, attribs)
          }
        case None =>
          println(s"${chipType} not found in any chip catalogs")
          return
      })
    }
  }

  private def includeOver(
    gatherer: GameGatherer,
    data: String,
    catalogs: ChipCatalogs
  ): Unit = {
    import toml.Value

    val parsed = {
      val tparsed = toml.Toml.parse(data)
      if (tparsed.isLeft) {
        println(s"game.over has failed to parse with error ${tparsed.left.get}")
        return
      }
      tparsed.right.get.values
    }

    // includes
    if (parsed.contains("includes")) {
      val tincs = parsed("includes").asInstanceOf[Value.Arr].values
      for (include <- tincs) {
        val table = include.asInstanceOf[Value.Tbl].values
        val incResource = table("resource").asInstanceOf[Value.Str].value

        FromResourceFile(incResource) match {
          case Some(d) => includeOver(gatherer, d, catalogs)
          case _ =>
            println("Include resource file ${incResource} not found")
            return
        }
      }
    }

    // buses this over has if any
    if (parsed.contains("buses")) {
      val tbuses = parsed("buses").asInstanceOf[Value.Arr].values
      for (bus <- tbuses) {
        val table = bus.asInstanceOf[Value.Tbl].values
        val name = table("name").asInstanceOf[Value.Str].value
        val bustype = table("type").asInstanceOf[Value.Str].value
        val count = tableIntGetOrElse(table, "count", 1)

        val attribs = table.filter(_._1 match {
          case "name" | "type" | "count" => false
          case _                         => true
        })

        gatherer.connections += (name -> BusDef(name, bustype, count, attribs))
      }
    }

    // interconnects this over has if any
    if (parsed.contains("interconnects")) {
      val tinterconnects =
        parsed("interconnects").asInstanceOf[Value.Arr].values
      for (interconnect <- tinterconnects) {
        val table = interconnect.asInstanceOf[Value.Tbl].values
        val name = table("name").asInstanceOf[Value.Str].value

        if (table.contains("connections")) {
          val connections =
            table("connections").asInstanceOf[Value.Arr].values

          val connecteds = {
            for (
              connection <- connections;
              c = connection.asInstanceOf[Value.Str].value
              if (gatherer.connections.contains(c))
            )
              yield gatherer.connections(c)
          }.toSeq
          val unconnecteds = {
            for (
              connection <- connections;
              c = connection.asInstanceOf[Value.Str].value
              if (!gatherer.connections.contains(c))
            )
              yield c
          }.toSeq

          val attribs = table.filter(_._1 match {
            case "name" | "connections" => false
            case _                      => true
          })
          gatherer.interconnects += InterconnectDef(
            name,
            connecteds,
            unconnecteds,
            attribs
          )
        } else
          println(s"Interconnect ${name} has no buses, ignoring")
      }
    }

    // extract non ram instances
    if (parsed.contains("instance")) {
      val tinstances = parsed("instance").asInstanceOf[Value.Arr].values
      parseInstances(tinstances, gatherer, catalogs)
    }
  }
  private def FromResourceFile(name: String): Option[String] = {
    println(s"Reading $name over")

    val path = Paths.get(s"src/main/resources/overs/${name}.over")
    if (!Files.exists(path.toAbsolutePath())) {
      println(s"${name} catalog at ${path} not found"); return None
    }

    val chipFile = path.toAbsolutePath().toFile()
    val source = scala.io.Source.fromFile(chipFile)

    Some(source.getLines mkString "\n")
  }

}
