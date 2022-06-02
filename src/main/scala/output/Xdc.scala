package output

import ikuy_utils._
import overlord.{DiffPinConstraint, Game, PinConstraint}

import scala.collection.mutable

object Xdc {
	def apply(game: Game): Unit = {
		val sb = new mutable.StringBuilder

		sb ++= "set_property CFGBVS VCCO [current_design]\n"
		sb ++= "set_property CONFIG_VOLTAGE 3.3 [current_design]\n"

		for {pinGrp <- game.pins
		     oconnected = game.connected.find(_.connectedTo(pinGrp))
		     if oconnected.nonEmpty
		     } {
			pinGrp.constraint match {
				case PinConstraint(pins, _, standard, names, directions, pullups) =>
					for (i <- pins.indices) {

						val dir = directions(i).toLowerCase match {
							case "input" | "in"   => "INPUT"
							case "output" | "out" => "OUTPUT"
							case "inout"          => "BIDIR"
							case _                => "ERROR"
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

				case DiffPinConstraint(pins,
				                       ports,
				                       standard,
				                       names,
				                       directions,
				                       pullups) => ???
			}
		}

		for (clk <- game.clocks) {
			sb ++=
			s"""set_property -dict {
				 |    PACKAGE_PIN ${clk.pin}
				 |    IOSTANDARD ${clk.standard}
				 |} [get_ports {${sanatizeIdent(clk.name)}}];
				 |""".stripMargin

			//@formatter:off
			sb ++=
			s"""create_clock -add -name ${sanatizeIdent(clk.name)} -period ${clk.period} -waveform ${clk.waveform} [get_ports {${sanatizeIdent(clk.name)}}]
				 |""".stripMargin
			//@formatter:on

		}

		Utils.writeFile(Game.outPath.resolve(s"${game.name}.xdc"), sb.result())
	}

	private def sanatizeIdent(in: String): String = {
		in.replaceAll("""->|\.""", "_")
	}
}
