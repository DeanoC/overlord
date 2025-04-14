package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.ChipDefinitionTrait

case class IoInstance(
    name: String,
    override val definition: ChipDefinitionTrait
) extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true
}

object IoInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, IoInstance] = {
    val io = IoInstance(ident, definition)
    io.mergeAllAttributes(attribs)
    Right(io)
  }
}
