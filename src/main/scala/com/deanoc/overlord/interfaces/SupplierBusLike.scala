package com.deanoc.overlord.Interfaces

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.Instances.ChipInstance

trait SupplierBusLike extends BusLike {
  override def isSupplier: Boolean = true

  def addFixedRelativeAddressConsumer(
      instance: ChipInstance,
      address: BigInt,
      size: BigInt
  ): Unit
  def addVariableAddressConsumer(instance: ChipInstance, size: BigInt): Unit
  def computeConsumerAddresses(): Unit
  def consumerVariant: Variant
  def getBusAlignment: BigInt
  def getBaseAddress: BigInt
  def fixedBaseBusAddress: Boolean
}
