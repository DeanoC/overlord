package overlord

import scala.collection.mutable.ArrayBuffer
import toml._

class SystemOnChip( override val name : String, override val isHard : Boolean,
                    val chips : List[Chip], 
                    val buses : List[Bus],                     
                    val interconnects : List[Interconnect])
extends Chip(name, isHard, "SystemOnChip")
{
  override def use(): Unit = {
    chips.foreach(_.use )
    buses.foreach(_.use )
    interconnects.foreach(_.use)
  }
}

object SystemOnChip {
  def parse(name : String, source : String ) : Option[SystemOnChip] = {
    val parsed = {
      val tparsed = toml.Toml.parse(source)
      if(tparsed.isLeft) return None
      tparsed.right.get
    }

    val isHard = parsed.values.contains("isHard") && parsed.values("isHard").asInstanceOf[toml.Value.Bool].value

    var chips = ArrayBuffer[Chip]()

    if(parsed.values.contains("chips")) {
      val tchips = parsed.values("chips").asInstanceOf[toml.Value.Arr].values

      for( chip <- tchips) {
        val table = chip.asInstanceOf[toml.Value.Tbl]
        val chipType = table.values("type").asInstanceOf[toml.Value.Str].value
        val name = if(!table.values.contains("name")) chipType
                   else table.values("name").asInstanceOf[toml.Value.Str].value
        val count = if(!table.values.contains("name")) 1 
                    else table.values("count").asInstanceOf[toml.Value.Num].value.toInt

        if(count == 1) chips += new Chip(name, isHard, chipType)
        else for( i <- (0 until count)) chips += new Chip(s"${name}${count}", isHard, chipType)
      }
    }

  
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
    if(parsed.values.contains("ram")) {
      val rams = parsed.values("ram").asInstanceOf[toml.Value.Arr].values
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

    var buses = ArrayBuffer[Bus]()

    // buses this SoC has if any
    if(parsed.values.contains("buses")) {
      val tbuses = parsed.values("buses").asInstanceOf[toml.Value.Arr].values
      for( bus <- tbuses) {
        val table = bus.asInstanceOf[toml.Value.Tbl]
        val name = table.values("name").asInstanceOf[toml.Value.Str].value
        val bustype = table.values("type").asInstanceOf[toml.Value.Str].value

        buses += {
          bustype match {
          case "internal" => new InternalBus(name, isHard)
          case "AXI4" => new Axi4Bus(name, isHard)
          case "_" => { println(s"Unknown bus type for ${name}"); new Bus(name, isHard) }
        }}
      }
    }


    var interconnects = ArrayBuffer[Interconnect]()
    // interconnects this SoC has if any
    if(parsed.values.contains("interconnects")) {
       val tinterconnects = parsed.values("interconnects").asInstanceOf[toml.Value.Arr].values
        for( interconnect <- tinterconnects) {
          val table = interconnect.asInstanceOf[toml.Value.Tbl]
          val name = table.values("name").asInstanceOf[toml.Value.Str].value
          
          if(table.values.contains("buses")) {
            val tbuses = table.values("buses").asInstanceOf[toml.Value.Arr].values

            var interbuses = ArrayBuffer[String]()

            for(bus <- tbuses) interbuses += bus.asInstanceOf[toml.Value.Str].value

            interconnects += new Interconnect(name, isHard, interbuses.toList)
          } else {
            println( s"Interconnect ${name} has no buses, ifnoring")
          }
        }
    }

    Some(new SystemOnChip(name, isHard, chips.toList, buses.toList, interconnects.toList))
  }
}