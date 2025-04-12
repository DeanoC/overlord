package com.deanoc.overlord.Instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class SwitchInstance(name: String, definition: ChipDefinitionTrait)
    extends ChipInstance {}

object SwitchInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, SwitchInstance] = {
    val chip = SwitchInstance(ident, definition)
    chip.mergeAllAttributes(attribs)
    Right(chip)
  }
}
