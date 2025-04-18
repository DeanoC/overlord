package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class IoInstance(
    name: String,
    override val definition: ChipDefinitionTrait,
    config: com.deanoc.overlord.config.IoConfig // Store the specific config
) extends ChipInstance {
  override def isVisibleToSoftware: Boolean = config.visible_to_software // Use visible_to_software from the specific config
}

object IoInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: com.deanoc.overlord.config.IoConfig // Accept IoConfig
  ): Either[String, IoInstance] = {
    try {
      // Create the IoInstance, passing the config
      val io = new IoInstance(name, definition, config)
      Right(io)
    } catch {
      case e: Exception =>
        Left(s"Error creating IoInstance: ${e.getMessage}")
    }
  }
}
