package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class SocInstance(name: String, definition: ChipDefinitionTrait)
    extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true
}

object SocInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: Map[String, Any] // Accept generic Map[String, Any] for config
  ): Either[String, SocInstance] = {
    val chip = SocInstance(name, definition)
    // Convert the Map[String, Any] config to Map[String, Variant] for merging
    val attribs = config.map { case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v) }
    chip.mergeAllAttributes(attribs)
    Right(chip)
  }
}
