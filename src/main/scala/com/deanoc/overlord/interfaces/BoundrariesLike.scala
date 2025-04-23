package com.deanoc.overlord.interfaces

import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.instances.HardwareInstance

trait BoundrariesLike extends ChipLike {
  def getBoundrariesStartingWith(startsWith: String): Seq[HardwareBoundrary]
  def getBoundrariesMatchingName(name: String): Seq[HardwareBoundrary]
}
