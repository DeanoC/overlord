package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class StorageInstance(name: String, private val defi: ChipDefinitionTrait)
    extends ChipInstance {
  override def definition: ChipDefinitionTrait = defi

  override def isVisibleToSoftware: Boolean = true

}

object StorageInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: Map[String, Any] // Accept generic Map[String, Any] for config
  ): Either[String, StorageInstance] = {
    val storage = StorageInstance(name, definition)
    // Convert the Map[String, Any] config to Map[String, Variant] for merging
    val attribs = config.map { case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v) }
    storage.mergeAllAttributes(attribs)
    Right(storage)
  }
}
