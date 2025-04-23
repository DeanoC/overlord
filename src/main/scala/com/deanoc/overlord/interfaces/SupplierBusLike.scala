package com.deanoc.overlord.interfaces

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.instances.HardwareInstance

trait SupplierBusLike extends BusLike {
  override def isSupplier: Boolean = true

  def addFixedRelativeAddressConsumer(
      instance: HardwareInstance,
      address: BigInt,
      size: BigInt
  ): Unit
  def addVariableAddressConsumer(instance: HardwareInstance, size: BigInt): Unit
  def computeConsumerAddresses(): Unit
  def consumerVariant: Variant
  def getBusAlignment: BigInt
  def getBaseAddress: BigInt
  def fixedBaseBusAddress: Boolean
}
