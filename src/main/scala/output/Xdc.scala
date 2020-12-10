package output

import java.nio.file.Path
import overlord.{DiffPinConstraint, Game, PinConstraint, Utils}

object Xdc {
	def apply(game: Game, out: Path): Unit = {
		val sb = new StringBuilder

		sb ++= "set_property CFGBVS VCCO [current_design]\n"
		sb ++= "set_property CONFIG_VOLTAGE 3.3 [current_design]\n"

		for {pinGrp <- game.pins
		     if game.connected.exists(_.connectsToInstance(pinGrp))
		     } {
			pinGrp.constraint match {
				case PinConstraint(pins, ports, names, directions, pullups) =>
					val standard = Utils.toString(pinGrp.attributes("standard"))
					for (i <- pins.indices) {

						val dir = directions(i) match {
							case "input"  => "INPUT"
							case "output" => "OUTPUT"
							case "inout"  => "BIDIR"
							case _        => "ERROR"
						}

						sb ++=
						s"""set_property -dict {
							 |    PACKAGE_PIN ${pins(i)}
							 |    IOSTANDARD $standard
							 |    PIO_DIRECTION ${dir}
							 |    PULLUP ${pullups(i)}
							 |} [get_ports {${sanatizeIdent(names(i))}}];
							 |""".stripMargin
					}

				case DiffPinConstraint(pins, ports, names, directions, pullups) =>
			}
		}

		for (clk <- game.clocks) {
			val standard = Utils.toString(clk.attributes("standard"))
			sb ++=
			s"""set_property -dict {
				 |    PACKAGE_PIN ${clk.pin}
				 |    IOSTANDARD $standard
				 |} [get_ports {${sanatizeIdent(clk.ident)}}];
				 |""".stripMargin

		}

		Utils.writeFile(out.resolve(s"${game.name}.xdc"), sb.result())
	}

	private def sanatizeIdent(in: String): String = {
		in.replaceAll("""->|\.""", "_")
	}
}
