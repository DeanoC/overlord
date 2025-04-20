package com.deanoc.overlord.instances

import com.deanoc.overlord.definitions.SoftwareDefinitionTrait

trait SoftwareInstance extends InstanceTrait {
  val folder: String = ""

  override def definition: SoftwareDefinitionTrait
}
