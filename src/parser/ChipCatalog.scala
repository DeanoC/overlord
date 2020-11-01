package overlord.parser

import toml._;
import java.nio.file.{ Files, Paths }
import java.io.File
import scala.collection.mutable.{ ArrayBuffer, HashMap }

case class ChipCatalog(val catalogName: String, val chips: List[ChipDef])

case class ChipCatalogs(val catalogs: Map[String, ChipCatalog]) {
  def FindChip(chipType: String): Option[ChipDef] = {
    val chipPath = chipType.split('.')
    if (chipPath.length < 2) return None

    catalogs.foreach {
      case (_, cat) =>
        (cat.chips.find { cc =>
          val split = cc.chipType.split('.')
          split(0) == chipPath(0) && split(1) == chipPath(1)
        }) match {
          case Some(cc) => return Some(cc)
          case _        =>
        }
    }

    println(s"${chipType} not found in any chip catalogs")
    return None
  }
}

object ChipCatalog {
  def FromFile(spath: String, name: String): Option[ChipCatalog] = {
    println(s"Reading $name catalog")

    val path = Paths.get(s"${spath}/${name}.toml")
    if (!Files.exists(path.toAbsolutePath())) {
      println(s"${name} catalog at ${path} not found"); return None
    }

    val chipFile = path.toAbsolutePath().toFile()
    val source = scala.io.Source.fromFile(chipFile)

    parse(name, source.getLines mkString "\n")
  }

  private def parse(name: String, source: String): Option[ChipCatalog] = {
    val parsed = {
      val tparsed = toml.Toml.parse(source)
      if (tparsed.isLeft) {
        println(
          s"$name catalog has failed to parse with error ${tparsed.left.get}"
        )
        return None
      }
      tparsed.right.get.values
    }

    var chips = ArrayBuffer[ChipDef]()

    if (parsed.contains("definition")) {
      val tchips = parsed("definition").asInstanceOf[toml.Value.Arr].values

      for (chip <- tchips) {
        val table = chip.asInstanceOf[toml.Value.Tbl].values
        val chipType = table("type").asInstanceOf[toml.Value.Str].value

        val attribs = table.filterNot(a => a._1 == "type")

        chips += ChipDef(chipType, attribs)
      }
    }

    Some(ChipCatalog(name, chips.toList))
  }
}
