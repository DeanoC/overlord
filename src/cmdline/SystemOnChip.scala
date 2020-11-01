package overlord

import scala.collection.mutable.ArrayBuffer
import java.nio.file.{ Files, Paths }
import java.io.File
import toml._
import overlord.parser._

case class SystemOnChip()

object SystemOnChip {

  def FromResourceFile(
      name: String,
      catalogs: ChipCatalogs
  ): Option[SystemOnChip] = {
    println(s"Reading $name soc")

    val path = Paths.get(s"src/main/resources/socs/${name}.toml")
    if (!Files.exists(path.toAbsolutePath())) {
      println(s"${name} soc at ${path} not found"); return None
    }

    val chipFile = path.toAbsolutePath().toFile()
    val source   = scala.io.Source.fromFile(chipFile)

    parse(name, source.getLines mkString "\n", catalogs)
  }

  def parse(
      name: String,
      source: String,
      catalogs: ChipCatalogs
  ): Option[SystemOnChip] = {
    val parsed = {
      val tparsed = toml.Toml.parse(source)
      if (tparsed.isLeft) {
        println(
          s"$name soc has failed to parse with error ${tparsed.left.get}"
        ); return None
      }
      tparsed.right.get
    }

    if (parsed.values.contains("instances")) {
      val tinstances =
        parsed.values("instances").asInstanceOf[toml.Value.Arr].values

      for (instance <- tinstances) {
        val table    = instance.asInstanceOf[toml.Value.Tbl]
        val chipType = table.values("type").asInstanceOf[toml.Value.Str].value
        val name =
          if (!table.values.contains("name")) chipType
          else table.values("name").asInstanceOf[toml.Value.Str].value

//        chips += new MiscChip(name, isHard, chipType)
      }
    }
    /*
    // cpu cluster if this SoC has if any
    if(parsed.values.contains("cpuclusters")) {
      val cpuClusters = parsed.values("cpuclusters").asInstanceOf[toml.Value.Arr].values

      for( cpuCluster <- cpuClusters) {
        val table = cpuCluster.asInstanceOf[toml.Value.Tbl]
        val name = table.values("name").asInstanceOf[toml.Value.Str].value
        val count = table.values("count").asInstanceOf[toml.Value.Num].value.toInt
        val arch = table.values("arch").asInstanceOf[toml.Value.Str].value
        val width = table.values("width").asInstanceOf[toml.Value.Num].value.toInt
        val localRam = if(!table.values.contains("localram")) None else
                      Some(table.values("localram").asInstanceOf[toml.Value.Str].value)

        chips += new CpuClusterChip(name, isHard, count, arch, width, localRam)
      }
    }
    // ram this SoC has if any
    if(parsed.values.contains("rams")) {
      val rams = parsed.values("rams").asInstanceOf[toml.Value.Arr].values
      for( ram <- rams) {
        val table = ram.asInstanceOf[toml.Value.Tbl]
        val size = table.values("size").asInstanceOf[toml.Value.Str].value
        val count = if(!table.values.contains("count")) 1 else
                      table.values("count").asInstanceOf[toml.Value.Num].value.toInt
        if(count > 1) {
          val prefixName = table.values("name").asInstanceOf[toml.Value.Str].value
          for(i <- (0 until count)) {
            chips += new RamChip(s"${prefixName}${i}", isHard, size)
          }
        } else {
          val name = table.values("name").asInstanceOf[toml.Value.Str].value
          chips += new RamChip(name, isHard, size)
        }
      }
    }

    Some(new SystemOnChip(name, isHard, chips.toList, buses.toList, interconnects.toList))
     */
    None
  }
}
