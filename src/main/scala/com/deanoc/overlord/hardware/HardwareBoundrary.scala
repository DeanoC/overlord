package com.deanoc.overlord.hardware
import com.deanoc.overlord.utils.{Utils, Variant, StringV, TableV}
import com.deanoc.overlord.config._
import com.deanoc.overlord.config.BitsDesc

case class HardwareBoundrary(
    name: String,
    width: BitsDesc,
    direction: WireDirection = WireDirection.InOut,
    knownWidth: Boolean = true
)

object HardwareBoundrary {
  // Updated factory method to use the pre-parsed BitsDesc
  def fromConfig(portConfig: BoundraryConfig): HardwareBoundrary = {
    HardwareBoundrary(
      name = portConfig.name,
      width = portConfig.bitsDesc,
      direction = portConfig.direction,
      knownWidth = true
    )
  }
}
