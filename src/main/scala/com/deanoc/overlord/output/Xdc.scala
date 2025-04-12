package com.deanoc.overlord.output

import scala.collection.mutable.StringBuilder

import com.deanoc.overlord.utils.{Utils, StringV}
import com.deanoc.overlord.{Project, PinConstraint, DiffPinConstraint}
import com.deanoc.overlord.Connections.{
  ConstantParameterType,
  FrequencyParameterType
}

object Xdc {
  def apply(game: Project): Unit = {
    val sb = new StringBuilder

    sb ++= "set_property CFGBVS VCCO [current_design]\n"
    sb ++= "set_property CONFIG_VOLTAGE 3.3 [current_design]\n"

    for {
      pinGrp <- game.pins
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

        case DiffPinConstraint(
              pins,
              ports,
              standard,
              names,
              directions,
              pullups
            ) =>
          ???
      }
    }

    for (clk <- game.clocks) {

      if (clk.pin != "INTERNAL") {
        sb ++=
          s"""set_property -dict {
					 |    PACKAGE_PIN ${clk.pin}
					 |    IOSTANDARD ${clk.standard}
					 |} [get_ports {${sanatizeIdent(clk.name)}}];
					 |""".stripMargin
      }

      val waveformTxt =
        if (clk.waveform.nonEmpty) s"-waveform ${clk.waveform}" else ""
      val pinOrPortTxt =
        if (clk.pin != "INTERNAL") s"get_ports {${sanatizeIdent(clk.name)}}"
        else s"get_pins {${sanatizeIdent(clk.name)}}"
      val periodTxt =
        if (clk.period > 0) s"-period ${clk.period}"
        else {
          val freq = {
            val const = game.constants.find(c =>
              c.parameter.name == clk.name.split("/").last
            )
            if (const.nonEmpty) const.get.parameter.parameterType match {
              case ConstantParameterType(_) =>
                println(
                  s"ERROR ${clk.name} must specify period or frequency using default 100Mhz"
                )
                "100 Mhz"
              case FrequencyParameterType(freq) => s"$freq Mhz"
            }
            else if (clk.frequency.isEmpty) {
              println(
                s"ERROR ${clk.name} must specify period or frequency using default 100Mhz"
              )
              "100Mhz"
            } else clk.frequency
          }
          val period = 1000.0 / Utils.toFrequency(StringV(freq))
          f"-period $period%.2f"
        }

      sb ++= s"""create_clock -add -name ${sanatizeIdent(
          clk.name
        )} $periodTxt $waveformTxt [$pinOrPortTxt]\n"""
    }

    Utils.writeFile(Project.outPath.resolve(s"${game.name}.xdc"), sb.result())
  }

  private def sanatizeIdent(in: String): String = {
    in.replaceAll("""->|\.""", "_")
  }
}
