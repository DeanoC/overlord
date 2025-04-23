package com.deanoc.overlord.interfaces

import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.instances.HardwareInstance

trait PortsLike extends ChipLike {
  def getPortsStartingWith(startsWith: String): Seq[HardwareBoundrary]
  def getPortsMatchingName(name: String): Seq[HardwareBoundrary]
}
