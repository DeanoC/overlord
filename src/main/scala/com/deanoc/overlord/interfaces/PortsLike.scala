package com.deanoc.overlord.Interfaces

import com.deanoc.overlord.Hardware.Port
import com.deanoc.overlord.Instances.ChipInstance

trait PortsLike extends ChipLike {
  def getPortsStartingWith(startsWith: String): Seq[Port]
  def getPortsMatchingName(name: String): Seq[Port]
}
