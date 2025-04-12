package com.deanoc.overlord.Instances

import com.deanoc.overlord.utils._
import com.deanoc.overlord.ChipDefinitionTrait

case class ClockInstance(
    name: String,
    override val definition: ChipDefinitionTrait
) extends ChipInstance {

  lazy val pin: String = Utils.lookupString(attributes, "pin", or = "INVALID")
  lazy val standard: String =
    Utils.lookupString(attributes, "standard", or = "LVCMOS33")
  lazy val period: Double = Utils.lookupDouble(attributes, "period", -1)
  lazy val frequency: String = Utils.lookupString(attributes, "frequency", "")
  lazy val waveform: String = Utils.lookupString(attributes, "waveform", "")

}

object ClockInstance {
  def apply(
      name: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, ClockInstance] = {

    if (!definition.attributes.contains("pin") && !attribs.contains("pin")) {
      Left(f"$name is a clock without a pin attribute")
    } else {
      val clock = ClockInstance(name, definition)
      clock.mergeAttribute(attribs, "pin")
      clock.mergeAttribute(attribs, "standard")
      clock.mergeAttribute(attribs, "period")
      clock.mergeAttribute(attribs, "waveform")

      Right(clock)
    }
  }
}
