package com.deanoc.overlord.output

import scala.collection.mutable

import com.deanoc.overlord.utils._
import com.deanoc.overlord.{Project}
import com.deanoc.overlord.connections.{
  InstanceLoc,
  WildCardConnectionPriority,
  Wire
}
import com.deanoc.overlord.instances.{
  ChipInstance,
  ClockInstance,
  PinGroupInstance
}

object Top {
  def apply(game: Project): Unit = {

    val sb = new mutable.StringBuilder()

    sb ++= s"module ${sanitizeIdent(game.name)}_top (\n"

    sb ++= writeTopWires(game.wires)

    sb ++= s"\n);\n"

    sb ++= writeChipToChipWires(game.wires)

    // instantiation
    for (instance <- game.setOfConnectedGateware) {
      sb ++=
        s"""
				 |  (*dont_touch = "true"*) ${instance.moduleName}""".stripMargin

      val merged = instance.finalParameterTable.toMap
        .filter(c => instance.parameterKeys.contains(c._1))

      if (merged.nonEmpty) {
        sb ++= " #(\n"
        var pcomma = ""
        for ((k, v) <- merged) {
          val sn = v match {
            case ArrayV(arr)       => "TODO"
            case BigIntV(bigInt)   => s"$bigInt"
            case BooleanV(boolean) => s"$boolean"
            case IntV(int)         => s"$int"
            case TableV(table)     => "TODO:"
            case StringV(string)   => s"'$string'"
            case DoubleV(dbl)      => s"$dbl"
          }

          sb ++= s"""$pcomma    .$k($sn)"""
          pcomma = ",\n"
        }
        sb ++= "  \n )"
      }

      sb ++= s" ${instance.name}(\n"

      sb ++= writeChipWires(instance, game.wires)

      sb ++=
        s"""
				 |  );
				 |""".stripMargin

    }

    sb ++= s"""endmodule\n"""

    Utils.writeFile(Project.outPath.resolve(game.name + "_top.v"), sb.result())
  }

  private def writeTopWires(wires: Seq[Wire]) = {

    val sb = new mutable.StringBuilder

    var comma = ""

    for (wire <- wires.filter(_.isStartPinOrClock)) {
      val s = writeTopWire(wire.startLoc, comma)
      if (s.nonEmpty) {
        sb ++= s
        comma = ",\n"
      }
    }

    for {
      wire <- wires.filterNot(_.priority == WildCardConnectionPriority())
      oloc = wire.findEndIsPinOrClock
      if oloc.nonEmpty
      loc = oloc.get
    } {
      val s = writeTopWire(loc, comma)
      if (s.nonEmpty) {
        sb ++= s
        comma = ",\n"
      }
    }

    sb.result()
  }

  def writeTopWire(loc: InstanceLoc, comma: String): String = {
    val sb = new mutable.StringBuilder

    if (loc.isClock) {
      val clk = loc.instance.asInstanceOf[ClockInstance]
      sb ++= s"$comma\tinput wire ${sanitizeIdent(loc.fullName)}"
    } else if (loc.port.nonEmpty) {
      val port = loc.port.get
      val dir = s"${port.direction}"
      val bits = if (port.width.singleBit) "" else s"${port.width.text}"
      sb ++= s"$comma\t$dir wire $bits ${sanitizeIdent(loc.fullName)}"
    }
    sb.result()
  }

  private def writeChipToChipWires(wires: Seq[Wire]): String = {
    val sb = new mutable.StringBuilder

    val c2cs = wires.filterNot(w => {
      w.isStartPinOrClock ||
      (w.findEndIsPinOrClock.nonEmpty && w.endLocs.length == 1)
    })

    for {
      wire <- c2cs
      if wire.startLoc.port.nonEmpty
    } {
      val port = wire.startLoc.port.get
      val width = if (!port.knownWidth) {
        wire.endLocs
          .map(i => if (i.port.nonEmpty) i.port.get.width.bitCount else 0)
          .max
      } else port.width.bitCount
      val bits = if (width == 1) "" else s"[${width - 1}:0]"
      sb ++= s"wire $bits ${sanitizeIdent(wire.startLoc.fullName)};\n"
    }
    sb.result()
  }

  private def writeChipWires(
      instance: ChipInstance,
      wires: Seq[Wire]
  ): String = {
    val sb = new mutable.StringBuilder

    var comma = ""
    val cs0 = wires.filter(_.startLoc.instance == instance)

    for { wire <- cs0 } {
      if (wire.startLoc.port.nonEmpty) {
        sb ++= s"$comma\t.${sanitizeIdent(wire.startLoc.port.get.name)}("
        val oloc = wire.endLocs.length match {
          case 0 => None
          case 1 => Some(wire.endLocs.head)
          case _ => wire.endLocs.find(_.instance == instance)
        }

        if (oloc.nonEmpty && (!oloc.get.isChip))
          sb ++= s"${sanitizeIdent(oloc.get.fullName)})"
        else
          sb ++= s"${sanitizeIdent(wire.startLoc.fullName)})"
        comma = ",\n"
      }
    }

    val cs1 = wires.diff(cs0).filter(_.endLocs.exists(_.instance == instance))
    for { wire <- cs1 } {
      val loc = wire.endLocs.find(_.instance == instance)
      if (loc.nonEmpty && loc.get.port.nonEmpty) {
        sb ++= s"$comma\t.${sanitizeIdent(loc.get.port.get.name)}("
        val inst = wire.startLoc.instance
        if (wire.startLoc.isClock) {
          val clk = inst.asInstanceOf[ClockInstance]
          if (clk.pin != "INTERNAL") sb ++= s"${sanitizeIdent(clk.name)})"
        } else if (wire.startLoc.isPin) {
          val pg = inst.asInstanceOf[PinGroupInstance]
          sb ++= s"${sanitizeIdent(pg.name)})"
        } else sb ++= s"${sanitizeIdent(wire.startLoc.fullName)})"

        comma = ",\n"
      }
    }

    sb.result()
  }

  private def sanitizeIdent(in: String): String = {
    in.replaceAll("""->|\.""", "_")
  }
}
