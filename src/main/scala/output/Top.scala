package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path

import overlord._

import scala.collection.mutable

object Top {
	def apply(game: Game, out: Path): Unit = {

		val setOfConnected: Set[Connected] = {
			for {c <- game.connections if c.isConnected} yield c.asConnected
		}.toSet

		val setOfGateware = {
			val setOfGateware = mutable.HashSet[GatewareInstance]()
			setOfConnected.foreach {
				case ConnectedBetween(_, m, s, _) =>
					if (m.isInstanceOf[GatewareInstance])
						setOfGateware += m.asInstanceOf[GatewareInstance]
					if (s.isInstanceOf[GatewareInstance])
						setOfGateware += s.asInstanceOf[GatewareInstance]

				case ConnectedConstant(_, _, t, _) =>
					if (t.isInstanceOf[GatewareInstance])
						setOfGateware += t.asInstanceOf[GatewareInstance]
			}
			setOfGateware.toSet
		}

		val sb = new StringBuilder()

		sb ++= s"""module ${sanatizeIdent(game.name)}_top (\n"""

		var comma = ""
		for (c <- game.constraintConnecteds) {
			val (dir, inst) = c.first match {
				case instance: ConstraintInstance => ("input wire   ", instance)
				case _                            =>
					c.second match {
						case instance: ConstraintInstance => ("output wire  ", instance)
						case _                            => ("inout  wire  ",
							c.first.asInstanceOf[ConstraintInstance])
					}
			} // TODO

			val bits = inst.constraint.constraintType match {
				case PinConstraint(pins)     => if (pins.length > 1) {
				} else "     "
				case DiffPinConstraint(pins) => if (pins.length > 1) {
				} else "     "
				case ClockPinConstraint()    => "     "
			}

			val name = sanatizeIdent(inst.ident)

			sb ++=
			s"""${comma}  $dir $bits${name}"""
			comma = ",\n"
		}

		sb ++= s"""\n);"""
/*
		for (c <- game.connections) {
			sb ++=
			s"""
				 |  wire ${sanatizeIdent(c.ident)};""".stripMargin
		}
		*/

		for (g <- setOfGateware) {
			comma = ""
			sb ++=
			s"""
				 |  ${g.definition.moduleName} ${g.ident}0(
				 |""".stripMargin

			for (port <- g.definition.ports) {
				sb ++=
				s"""$comma    .${sanatizeIdent(port)}("""

				val cons = setOfConnected.filter(
					_.unconnected match {
						case UnconnectedBetween(d, m, s)  => g.matchIdent(m) ||
						                                     g.matchIdent(s)
						case UnconnectedConstant(d, c, t) => g.matchIdent(t)
						case _                            => false
					})

				for {sc <- cons} {
					sc.unconnected match {
						case u: UnconnectedBetween  =>
							if (u.first.split('.').last == port)
								sb ++= s"${sanatizeIdent(u.second)}"
							else if (u.second.split('.').last == port)
								sb ++= s"${sanatizeIdent(u.first)}"
						case c: UnconnectedConstant =>
							if (c.to.split('.').last == port)
								sb ++= s"${sanatizeIdent(c.to)}"
						case _                      =>
					}
				}

				/*
				val con = game.constraintConnecteds.find(_.first.matchIdent(port))
				con match {
					case Some(v) => sb ++= s"""${sanatizeIdent(port)}"""
					case None        =>
						sb ++= (s"Cannot find connection for ${g.ident}.${port}")
				}*/

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
