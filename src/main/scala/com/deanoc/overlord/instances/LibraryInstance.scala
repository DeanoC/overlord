package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.SoftwareDefinitionTrait

case class LibraryInstance(
    override val name: String,
    override val definition: SoftwareDefinitionTrait,
    config: com.deanoc.overlord.config.LibraryConfig // Store the specific config
) extends SoftwareInstance {
  override val folder = "libs"
  // Dependencies are now available via config.dependencies
}

object LibraryInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: SoftwareDefinitionTrait,
      config: com.deanoc.overlord.config.LibraryConfig // Accept LibraryConfig
  ): Either[String, LibraryInstance] = {
    try {
      // Create the LibraryInstance, passing the config
      val sw = new LibraryInstance(name, definition, config)
      Right(sw)
    } catch {
      case e: Exception =>
        Left(s"Error creating LibraryInstance: ${e.getMessage}")
    }
  }
}
