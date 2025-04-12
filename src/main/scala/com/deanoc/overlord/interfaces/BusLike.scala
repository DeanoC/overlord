package com.deanoc.overlord.Interfaces

trait BusLike extends ChipLike {
  def isHardware: Boolean
  def isSupplier: Boolean
  def getPrefix: String
}
