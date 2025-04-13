package com.deanoc.overlord.interfaces

import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.instances.ChipInstance

trait PortsLike extends ChipLike {
  def getPortsStartingWith(startsWith: String): Seq[Port]
  def getPortsMatchingName(name: String): Seq[Port]
}
