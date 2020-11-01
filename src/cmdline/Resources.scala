package overlord.cmdline

import toml._;
import java.nio.file.{ Files, Path, Paths }
import java.io.File
import scala.collection.mutable.{ ArrayBuffer, HashMap }
import overlord.parser._

case class Resources(path: String = "src/main/resources/") {
  def loadCatalogs(): Map[String, ChipCatalog] = {
    val parsed = loadToToml(Paths.get(s"${path}/catalogs.toml").toAbsolutePath)
    if (
      !parsed.values.contains("resources") || !parsed
        .values("resources")
        .isInstanceOf[toml.Value.Arr]
    ) Map[String, ChipCatalog]()
    else {
      val resources =
        parsed.values("resources").asInstanceOf[toml.Value.Arr].values
      val resourceMaps = HashMap[String, ChipCatalog]()
      for (resource <- resources) {
        val name    = resource.asInstanceOf[toml.Value.Str].value
        val catalog = ChipCatalog.FromFile(s"$path/catalogs/", s"$name")
        catalog match {
          case Some(c) => resourceMaps += (name -> c)
          case None    =>
        }
      }
      resourceMaps toMap
    }
  }

  def loadBoards(catalogs: ChipCatalogs): Map[String, Board] = {
    val parsed = loadToToml(Paths.get(s"${path}/boards.toml").toAbsolutePath)
    if (
      !parsed.values.contains("resources") || !parsed
        .values("resources")
        .isInstanceOf[toml.Value.Arr]
    ) Map[String, Board]()
    else {
      val resources =
        parsed.values("resources").asInstanceOf[toml.Value.Arr].values
      val resourceMaps = HashMap[String, Board]()
      for (resource <- resources) {
        val name  = resource.asInstanceOf[toml.Value.Str].value
        val board = Board.FromFile(s"$path/boards/", name, catalogs)
        board match {
          case Some(b) => resourceMaps += (name -> b)
          case None    =>
        }
      }
      resourceMaps toMap
    }
  }

  private def loadToToml(absolutePath: Path): toml.Value.Tbl = {
    if (!Files.exists(absolutePath)) {
      println(s"$absolutePath does't not exists");
      return toml.Value.Tbl(Map[String, toml.Value]())
    }

    val file    = absolutePath.toFile()
    val source  = scala.io.Source.fromFile(file).getLines mkString "\n"
    val tparsed = toml.Toml.parse(source)
    if (tparsed.isLeft) {
      println(
        s"${absolutePath} has failed to parse with error ${tparsed.left.get}"
      ); toml.Value.Tbl(Map[String, toml.Value]())
    } else tparsed.right.get
  }
}
