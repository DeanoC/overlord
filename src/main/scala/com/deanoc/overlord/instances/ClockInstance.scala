package com.deanoc.overlord.instances

import com.deanoc.overlord.utils._
import com.deanoc.overlord.ChipDefinitionTrait

case class ClockInstance(
    name: String,
    override val definition: ChipDefinitionTrait,
    config: com.deanoc.overlord.config.ClockConfig // Store the specific config
) extends ChipInstance {

  lazy val pin: String = Utils.lookupString(attributes, "pin", or = "INVALID")
  lazy val standard: String =
    Utils.lookupString(attributes, "standard", or = "LVCMOS33")
  lazy val period: Double = Utils.lookupDouble(attributes, "period", -1)
  lazy val frequency: String = config.frequency // Use frequency from the specific config
  lazy val waveform: String = Utils.lookupString(attributes, "waveform", "")

}

object ClockInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: com.deanoc.overlord.config.ClockConfig // Accept ClockConfig
  ): Either[String, ClockInstance] = {
    try {
      // Create the ClockInstance, passing the config
      val clock = new ClockInstance(name, definition, config)
      Right(clock)
    } catch {
      case e: Exception =>
        Left(s"Error creating ClockInstance: ${e.getMessage}")
    }
  }
}
