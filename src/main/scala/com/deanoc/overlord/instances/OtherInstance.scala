package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.definitions.HardwareDefinition

case class OtherInstance(name: String, definition: HardwareDefinition)
    extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true

}

object OtherInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: HardwareDefinition,
      config: Map[String, Any] // Accept generic Map[String, Any] for config
  ): Either[String, OtherInstance] = {
    val other = OtherInstance(name, definition)
    // Convert the Map[String, Any] config to Map[String, Variant] for merging
    val attribs = config.map { case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v) }
    other.mergeAllAttributes(attribs)
    Right(other)
  }
}
