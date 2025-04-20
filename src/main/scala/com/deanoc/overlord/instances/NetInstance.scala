package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.definitions.ChipDefinitionTrait

case class NetInstance(
    name: String,
    override val definition: ChipDefinitionTrait
) extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true
}

object NetInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: Map[String, Any] // Accept generic Map[String, Any] for config
  ): Either[String, NetInstance] = {
    val net = NetInstance(name, definition)
    // Convert the Map[String, Any] config to Map[String, Variant] for merging
    val attribs = config.map { case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v) }
    net.mergeAllAttributes(attribs)
    Right(net)
  }
}
