package com.deanoc.overlord.Instances

import com.deanoc.overlord.SoftwareDefinitionTrait

trait SoftwareInstance extends InstanceTrait {
  val folder: String = ""

  override def definition: SoftwareDefinitionTrait
}
