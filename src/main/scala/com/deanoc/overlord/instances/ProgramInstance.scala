package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.definitions.SoftwareDefinitionTrait

case class ProgramInstance(
    name: String,
    override val definition: SoftwareDefinitionTrait,
//    config: com.deanoc.overlord.config.ProgramConfig // Store the specific config
) extends SoftwareInstance {
  override val folder = "programs"
  // Dependencies are now available via config.dependencies
}

object ProgramInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: SoftwareDefinitionTrait,
//      config: com.deanoc.overlord.config.ProgramConfig // Accept ProgramConfig
  ): Either[String, ProgramInstance] = {

    Left("TODO: Implement ProgramInstance.apply method")
    /*
    try {
      // Create the ProgramInstance, passing the config
      val sw = new ProgramInstance(name, definition, config)
      Right(sw)
    } catch {
      case e: Exception =>
        Left(s"Error creating ProgramInstance: ${e.getMessage}")
    }
    */
  }
}
