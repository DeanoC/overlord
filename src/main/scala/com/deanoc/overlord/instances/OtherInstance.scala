package com.deanoc.overlord.Instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class OtherInstance(name: String, definition: ChipDefinitionTrait)
    extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true

}

object OtherInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, OtherInstance] = {
    val other = OtherInstance(ident, definition)
    other.mergeAllAttributes(attribs)
    Right(other)
  }
}
