package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path
import overlord.Connections.{Connected, ConnectedBetween, ConnectedConstant, InstanceLoc, Unconnected, Wire, Wires}
import overlord.Gateware.{Gateware, Port, WireDirection}
import overlord.Instances.{BoardInstance, ClockInstance, Instance, PinGroupInstance}
import overlord._
import toml.Value

import scala.collection.mutable
import scala.util.{Failure, Success, Try}

object Top {

	type TopPort = (Instance, WireDirection, Port)

	def apply(game: Game, out: Path): Unit = {

		val dm = game.distanceMatrix

		val setOfConnected = game.connections
			.filter(_.isConnected)
			.map(_.asConnected).toSet

		val setOfGateware = {
			val setOfGateware = mutable.HashSet[Instance]()
			setOfConnected.foreach { c =>
				if (c.first.nonEmpty && c.first.get.instance.isGateware)
					setOfGateware += c.first.get.instance
				if (c.second.nonEmpty && c.second.get.instance.isGateware)
					setOfGateware += c.second.get.instance
			}
			setOfGateware.toSet
		}

		val connectionMask = Array.fill[Boolean](dm.dim)(elem = false)
		for {connected <- setOfConnected
		     (sp, ep) = dm.indicesOf(connected)} {
			connectionMask(sp) = true
			dm.routeBetween(sp, ep).foreach(connectionMask(_) = true)
		}
		dm.removeSelfLinks()
		dm.instanceMask(connectionMask)

		val wires = Wires(dm, setOfConnected.toSeq)

		val sb = new StringBuilder()

		sb ++=
		s"""module ${sanatizeIdent(game.name)}_top (\n"""

		sb ++= writeTopWires(wires)

		sb ++= s"""\n);\n"""

		sb ++= writeChipToChipWires(wires)

		// instantiation
		for ((instance, gw) <- setOfGateware.map(
			i => (i, i.definition.gateware.get))) {

			sb ++=
			s"""
				 |  ${instance.ident}""".stripMargin

			val merged = {
				val m = game.constants
					.map(_.asParameter)
					.fold(gw.parameters)((o, n) => o ++ n)
				m.filter(c => gw.verilog_parameters.contains(c._1))
			}

			if (merged.nonEmpty) {
				sb ++= " #(\n"
				var pcomma = ""
				for ((k, v) <- merged) {
					val sn = Try {
						v.value.toLong
					} match {
						case Failure(_)     => s"'${v.value.toString()}'"
						case Success(value) => s"$value"
					}

					sb ++= s"""$pcomma    .${k}($sn)"""
					pcomma = ",\n"
				}
				sb ++= "  \n )"
			}

			sb ++= s" ${instance.ident}(\n"

			sb ++= writeChipWires(instance, wires)

			sb ++=
			s"""
				 |  );
				 |""".stripMargin

		}

		sb ++= s"""endmodule\n"""

		writeFile(out.resolve(game.name + "_top.v"), sb.result())
	}

	def writeTopWire(loc: InstanceLoc, comma: String): String = {
		val sb = new StringBuilder

		if (loc.isClock) {
			val clk = loc.instance.asInstanceOf[ClockInstance]
			sb ++= s"$comma\tinput wire ${sanatizeIdent(loc.fullName)}"
		} else if (loc.port.nonEmpty) {
			val port = loc.port.get
			val wdir = port.direction
			val dir  = s"${wdir}"
			val bits = if (port.width.singleBit) "" else s"${port.width.text}"
			sb ++= s"$comma\t$dir wire $bits ${sanatizeIdent(port.name)}"
		}
		sb.result()
	}


	private def writeTopWires(wires: Seq[Wire]) = {

		val sb: StringBuilder = new StringBuilder

		var comma = ""

		for {wire <- wires.filter(w => w.startLoc.instance
			                               .isInstanceOf[PinGroupInstance] ||
		                               w.startLoc.instance
			                               .isInstanceOf[ClockInstance])
		     loc = wire.startLoc} {
			val s = writeTopWire(loc, comma)
			if (s.nonEmpty) {
				sb ++= s
				comma = ",\n"
			}
		}

		for {wire <- wires
		     oloc = wire.endLocs
			     .find(il => il.instance.isInstanceOf[PinGroupInstance] ||
			                 il.instance.isInstanceOf[ClockInstance])
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

		val c2cs = wires.filter(
			w => {
				if (!(w.startLoc.instance.isInstanceOf[PinGroupInstance] ||
				      w.startLoc.instance.isInstanceOf[ClockInstance])) {
					var result = false
					for {
						ew <- w.endLocs
						if !(ew.instance.isInstanceOf[PinGroupInstance] ||
						     ew.instance.isInstanceOf[ClockInstance])
					} result = true

					result
				} else false
			})

		for {wire <- c2cs
		     if wire.startLoc.port.nonEmpty} {
			val port = wire.startLoc.port.get
			val bits = if (port.width.singleBit) "" else s"${port.width.text}"
			sb ++= s"wire $bits ${sanatizeIdent(wire.startLoc.fullName)};\n"
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
				sb ++= s"$comma\t.${sanatizeIdent(wire.startLoc.port.get.name)}("
				val loc = wire.endLocs.length match {
					case 0 => None
					case 1 => Some(wire.endLocs.head)
					case _ => wire.endLocs.find(_.instance == instance)
				}

				if (loc.nonEmpty) {
					if (wire.startLoc.isClock) {
						val clk = wire.startLoc.instance.asInstanceOf[ClockInstance]
						sb ++= s"${sanatizeIdent(clk.ident)})"
					} else if (wire.startLoc.isPin) {
						val pg = wire.startLoc.instance.asInstanceOf[PinGroupInstance]
						sb ++= s"${sanatizeIdent(pg.ident)})"
					} else if (loc.get.port.nonEmpty) {
						sb ++= s"${sanatizeIdent(wire.startLoc.fullName)})"
					} else
						sb ++= s"${sanatizeIdent(wire.endLocs.head.fullName)})"
				} else
					sb ++= s"${sanatizeIdent(wire.startLoc.fullName)})"
				comma = ",\n"
			}
		}

		val cs1 = wires.diff(cs0).filter(_.endLocs.exists(_.instance == instance))
		for {wire <- cs1} {
			val loc = wire.endLocs.find(_.instance == instance)
			if (loc.nonEmpty && loc.get.port.nonEmpty) {
				sb ++= s"$comma\t.${sanatizeIdent(loc.get.port.get.name)}("
				if (wire.startLoc.isClock) {
					val clk = wire.startLoc.instance.asInstanceOf[ClockInstance]
					sb ++= s"${sanatizeIdent(clk.ident)})"
				} else if (wire.startLoc.isPin) {
					val pg = wire.startLoc.instance.asInstanceOf[PinGroupInstance]
					sb ++= s"${sanatizeIdent(pg.ident)})"
				} else sb ++= s"${sanatizeIdent(wire.startLoc.fullName)})"

				comma = ",\n"
			}
		}

		sb.result()
	}

	private def sanatizeIdent(in: String): String = {
		in.replaceAll("""->|\.""", "_")
	}

	private def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}

}
