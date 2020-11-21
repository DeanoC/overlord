package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path

import overlord.Connections.{
	Connected, ConnectedBetween, ConnectedConstant,
	Unconnected
}
import overlord.Instances.{ConstraintInstance, Instance}
import overlord._

import scala.collection.mutable

object Top {

	private def pinCount(name: String,
	                     constraints: Map[String, ConstraintType]): Int = {
		if (constraints.contains(name))
			constraints(name) match {
				case PinConstraint(pins)     =>
					if (pins.length > 1) return pins.length
				case DiffPinConstraint(pins) =>
					if (pins.length > 1) return pins.length
				case _                       =>
			}
		Int.MaxValue
	}

	def apply(game: Game, out: Path): Unit = {

		val setOfConnected = game.connections
			.filter(_.isConnected)
			.map(_.asConnected).toSet

		val setOfGateware = {
			val setOfGateware = mutable.HashSet[Instance]()
			setOfConnected.foreach { c =>
				if (c.first.nonEmpty && c.first.get.isGateware)
					setOfGateware += c.first.get
				if (c.second.nonEmpty && c.second.get.isGateware)
					setOfGateware += c.second.get
			}
			setOfGateware.toSet
		}

		val usedConstraintsWithDir  = mutable.HashMap[String, Int]()
		val usedConstraintsWithType = mutable.HashMap[String, ConstraintType]()

		for (c <- game.constraintConnecteds) {
			val (id, ctype, dir) = {
				if (c.first.nonEmpty &&
				    c.first.get.isInstanceOf[ConstraintInstance]) {
					val ci = c.first.get.asInstanceOf[ConstraintInstance]
					(sanatizeIdent(ci.ident), ci.constraintType, 1)
				} else if (c.second.nonEmpty &&
				           c.second.get.isInstanceOf[ConstraintInstance]) {
					val ci = c.second.get.asInstanceOf[ConstraintInstance]
					(sanatizeIdent(ci.ident), ci.constraintType, 2)
				} else {
					println("ERROR not a constraint")
					return
				}
			}
			usedConstraintsWithType(id) = ctype
			if (usedConstraintsWithDir.contains(id))
				usedConstraintsWithDir(id) |= dir
			else
				usedConstraintsWithDir(id) = dir
		}
		usedConstraintsWithDir.mapValuesInPlace((_, v) => math.min(v, 3))

		// we want to combine all connection ports into
		// a single wire

		// ordered 1st name =
		// [ 2nd name, first, chipToChip, current count, max count)
		val wiring =
			mutable.HashMap[String, (String, Boolean, Boolean, Int, Int)]()

		for (p <- setOfConnected) yield {
			val fn = sanatizeIdent(p.firstFullName)
			val sn = sanatizeIdent(p.secondFullName)

			val pinCnt = Math.min(
				pinCount(fn, usedConstraintsWithType.toMap),
				pinCount(sn, usedConstraintsWithType.toMap))

			if (p.isChipToChip) {
				wiring(fn) = (sn, true, true, 0, pinCnt)
				wiring(sn) = (fn, false, true, 0, pinCnt)

			} else if (p.isPortToChip) {
				wiring(fn) = (sn, true, false, 0, pinCnt)
				wiring(sn) = (fn, false, false, 0, pinCnt)
			} else if (p.isChipToPort) {
				wiring(fn) = (sn, false, false, 0, pinCnt)
				wiring(sn) = (fn, true, false, 0, pinCnt)
			}
		}

		val sb = new StringBuilder()

		sb ++=
		s"""module ${sanatizeIdent(game.name)}_top (\n"""


		var comma = ""
		for ((id, dir) <- usedConstraintsWithDir) {
			val dirString = dir match {
				case 1 => "input wire"
				case 2 => "output wire"
				case 3 => "inout wire"
				case _ => println(s"${id} has no direction?"); ""
			}

			val bits = usedConstraintsWithType(id) match {
				case PinConstraint(pins)     =>
					if (pins.length > 1) s"[${pins.length - 1}:0] "
					else ""
				case DiffPinConstraint(pins) =>
					if (pins.length > 1) s"[${pins.length - 1}]"
					else ""
				case ClockPinConstraint()    => ""
			}

			val name = sanatizeIdent(id)
			sb ++= s"""${comma} $dirString $bits${name}"""
			comma = ",\n"
		}


		sb ++= s"""\n);\n"""

		var pcomma = ""

		for ((k, v) <- wiring; if v._2; if v._3)
			sb ++= s" wire ${sanatizeIdent(k)};\n"

		// instantiation
		for ((gw, d) <- setOfGateware.map(
			gw => (gw, gw.definition.gateware.get))) {

			sb ++=
			s"""
				 |  ${d.moduleName}""".stripMargin

			val parameters = d.parameters
			val merged     = game.constants
				.map(_.asParameter)
				.fold(parameters)((o, n) => o ++ n)

			if (parameters.nonEmpty) sb ++= " #("
			for (param <- parameters) {
				val sn = sanatizeIdent(merged(param._1))
				sb ++=
				s"""
					 |    .${param._1}($sn)$pcomma
					 |"""
					.stripMargin
				pcomma = "\n,"
			}
			if (parameters.nonEmpty) sb ++= " )"

			sb ++= s" ${gw.ident}0(\n"

			var comma = ""

			for (port <- d.ports) {
				sb ++= s"""$comma    .${sanatizeIdent(port)}("""

				val found = wiring.contains(sanatizeIdent(s"${gw.ident}.$port"))
				if(found) {
					val (id, (c, max)): (String, (Int, Int)) = {
						val connections =
							(for (p <- setOfConnected) yield {
								val fn = sanatizeIdent(p.firstFullName)
								val sn = sanatizeIdent(p.secondFullName)
								//							println(s"${d.moduleName} ${gw.ident} $port ${fn} ${sn}")



								if (p.firstLastName == port) {
									Some(sn, (wiring(sn)._4, wiring(sn)._5))
								} else if (p.secondLastName == port) {
									Some(fn, (wiring(fn)._4, wiring(fn)._5))
								} else None
							}
								).flatten

						if (connections.nonEmpty)
							connections.head
						else {
							println(s"$port has no connections")
							("0", (0, Int.MaxValue))
						}
					}

					val sanId = sanatizeIdent(id)
					if (wiring.contains(sanId) &&
					    wiring(sanId)._3) {
						val (nm, first) = (wiring(sanId)._1, wiring(sanId)._2)
						if (first) sb ++= sanId
						else sb ++= nm
					} else sb ++= sanId

					if (max > 1 && max != Int.MaxValue) {
						sb ++= s"[$c]"
					}

					if (c >= max) {
						println(s"$port is using too many bits!")
					}
					//				wiring(sanId)bitCounter(port) = (c + 1, max)
				} else sb ++= "0"
				sb ++= s")"
				comma = ",\n"
			}

			sb ++=
			s"""
				 |  );
				 |""".stripMargin

		}

		sb ++= s"""endmodule\n"""

		writeFile(out.resolve(game.name + "_top.v"), sb.result())

	}

	private def sanatizeIdent(in: String): String = {
		in
			.replaceAll("""->|\.""", "_")
	}

	private def ensureDirectories(path: Path): Unit = {
		val directory = path.toFile
		if (!directory.exists()) {
			directory.mkdirs()
		}
	}

	private def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}

}
