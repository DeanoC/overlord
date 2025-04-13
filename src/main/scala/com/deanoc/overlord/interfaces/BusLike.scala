package com.deanoc.overlord.interfaces

trait BusLike extends ChipLike {
  def isHardware: Boolean
  def isSupplier: Boolean
  def getPrefix: String
}
