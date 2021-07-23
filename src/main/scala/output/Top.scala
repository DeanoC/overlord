package output

import ikuy_utils._

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path
import overlord.Connections.{
	InstanceLoc, WildCardConnectionPriority, Wire,
	Wires
}
import overlord.Gateware.{Port, WireDirection}
import overlord.Instances.{ClockInstance, Instance, PinGroupInstance}
import overlord._

import scala.collection.mutable
import scala.util.{Failure, Success, Try}

object Top {
	def apply(game: Game, out: Path): Unit = {

		val sb = new StringBuilder()

		sb ++= s"module ${sanitizeIdent(game.name)}_top (\n"

		sb ++= writeTopWires(game.wires)

		sb ++= s"\n);\n"

		sb ++= writeChipToChipWires(game.wires)

		// instantiation
		for ((instance, gw) <- game.setOfGateware.map(
			i => (i, i.definition.gateware.get))) {

			sb ++=
			s"""
				 |  (*dont_touch = "true"*) ${instance.ident}""".stripMargin

			val merged = game.constants
				.map(_.asParameter)
				.fold(gw.parameters)((o, n) => o ++ n)
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

			sb ++= s" ${instance.ident}(\n"

			sb ++= writeChipWires(instance, game.wires)

			sb ++=
			s"""
				 |  );
				 |""".stripMargin

		}

		sb ++= s"""endmodule\n"""

		Utils.writeFile(out.resolve(game.name + "_top.v"), sb.result())
	}

	def writeTopWire(loc: InstanceLoc, comma: String): String = {
		val sb = new StringBuilder

		if (loc.isClock) {
			val clk = loc.instance.asInstanceOf[ClockInstance]
			sb ++= s"$comma\tinput wire ${sanitizeIdent(loc.fullName)}"
		} else if (loc.port.nonEmpty) {
			val port = loc.port.get
			val dir  = s"${port.direction}"
			val bits = if (port.width.singleBit) "" else s"${port.width.text}"
			sb ++= s"$comma\t$dir wire $bits ${sanitizeIdent(loc.fullName)}"
		}
		sb.result()
	}


	private def writeTopWires(wires: Seq[Wire]) = {

		val sb: StringBuilder = new StringBuilder

		var comma = ""

		for (wire <- wires.filter(_.isStartPinOrClock)) {
			val s = writeTopWire(wire.startLoc, comma)
			if (s.nonEmpty) {
				sb ++= s
				comma = ",\n"
			}
		}

		for {wire <- wires.filterNot(_.priority == WildCardConnectionPriority())
		     oloc = wire.findEndIsPinOrClock
		     if oloc.nonEmpty
		     loc = oloc.get} {
			val s = writeTopWire(loc, comma)
			if (s.nonEmpty) {
				sb ++= s
				comma = ",\n"
			}
		}

		sb.result()
	}

	private def writeChipToChipWires(wires: Seq[Wire]): String = {
		val sb: StringBuilder = new StringBuilder

		val c2cs = wires.filterNot(
			w => {
				w.isStartPinOrClock ||
				(w.findEndIsPinOrClock.nonEmpty && w.endLocs.length == 1)
			})

		for {wire <- c2cs
		     if wire.startLoc.port.nonEmpty} {
			val port = wire.startLoc.port.get
			val bits = if (port.width.singleBit) "" else s"${port.width.text}"
			sb ++= s"wire $bits ${sanitizeIdent(wire.startLoc.fullName)};\n"
		}
		sb.result()
	}

	private def writeChipWires(instance: Instance,
	                           wires: Seq[Wire]): String = {
		val sb: StringBuilder = new StringBuilder

		var comma = ""
		val cs0   = wires.filter(_.startLoc.instance == instance)

		for {wire <- cs0} {
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
		for {wire <- cs1} {
			val loc = wire.endLocs.find(_.instance == instance)
			if (loc.nonEmpty && loc.get.port.nonEmpty) {
				sb ++= s"$comma\t.${sanitizeIdent(loc.get.port.get.name)}("
				val inst = wire.startLoc.instance
				if (wire.startLoc.isClock) {
					val clk = inst.asInstanceOf[ClockInstance]
					sb ++= s"${sanitizeIdent(clk.ident)})"
				} else if (wire.startLoc.isPin) {
					val pg = inst.asInstanceOf[PinGroupInstance]
					sb ++= s"${sanitizeIdent(pg.ident)})"
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
