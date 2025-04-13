package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class SocInstance(name: String, definition: ChipDefinitionTrait)
    extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true
}

object SocInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, SocInstance] = {
    val chip = SocInstance(ident, definition)
    chip.mergeAllAttributes(attribs)
    Right(chip)
  }
}
