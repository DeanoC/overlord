package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class SwitchInstance(name: String, definition: ChipDefinitionTrait)
    extends ChipInstance {}

object SwitchInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: Map[String, Any] // Accept generic Map[String, Any] for config
  ): Either[String, SwitchInstance] = {
    val chip = SwitchInstance(name, definition)
    // Convert the Map[String, Any] config to Map[String, Variant] for merging
    val attribs = config.map { case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v) }
    chip.mergeAllAttributes(attribs)
    Right(chip)
  }
}
