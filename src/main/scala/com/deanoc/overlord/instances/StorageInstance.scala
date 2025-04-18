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
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, StorageInstance] = {
    val storage = StorageInstance(ident, definition)
    storage.mergeAllAttributes(attribs)
    Right(storage)
  }
}
