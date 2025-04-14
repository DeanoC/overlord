package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class GraphicInstance(name: String, private val defi: ChipDefinitionTrait)
    extends ChipInstance {
  override def definition: ChipDefinitionTrait = defi

  override def isVisibleToSoftware: Boolean = true

}

object GraphicInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, GraphicInstance] = {
    val graphic = GraphicInstance(ident, definition)
    graphic.mergeAllAttributes(attribs)
    Right(graphic)
  }
}
