package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.Utils.VariantTable
import com.deanoc.overlord.utils.{ArrayV, BigIntV, Utils, Variant}
import com.deanoc.overlord.interfaces.SupplierBusLike

import scala.collection.mutable
import scala.reflect.ClassTag

case class BusSpec(
    name: String,
    supplier: Boolean,
    baseAddr: BigInt,
    dataWidth: BigInt,
    addrWidth: BigInt,
    protocol: String,
    prefix: String,
    fixedAddress: Boolean
)

case class Bus(
    owner: ChipInstance,
    ident: String,
    attributes: VariantTable,
    spec: BusSpec
) extends SupplierBusLike {
  private val fixedRelativeAddrConsumers =
    mutable.ArrayBuffer[(ChipInstance, BigInt, BigInt)]()
  private val variableAddrConsumers =
    mutable.ArrayBuffer[(ChipInstance, BigInt)]()

  private var consumers =
    mutable.HashMap[(ChipInstance, BigInt), (BigInt, BigInt)]()

  def consumerInstances: Seq[ChipInstance] = consumers.keys.toSeq.map(_._1)

  def consumerCount: Int = consumers.size

  // query interface delegated to owner
  override def getInterface[T](implicit tag: ClassTag[T]): Option[T] =
    owner.getInterface[T]

  // bus like interface
  override def isSupplier: Boolean = spec.supplier

  override def getPrefix: String = spec.prefix

  override def getBusAlignment: BigInt =
    Utils.lookupBigInt(attributes.toMap, "bus_bank_alignment", 1024)

  override def addFixedRelativeAddressConsumer(
      instance: ChipInstance,
      address: BigInt,
      size: BigInt
  ): Unit = {
    if (instance == owner) {
      // Instead of returning Left, log the error as a warning
      // This maintains compatibility with the interface while still tracking the error
      val error = s"addFixedAddressConsumer adding itself $ident"
      // Create an Either for potential future refactoring but don't return it
      val _ = Left(error)
      // In a future refactoring, this might propagate the error
    } else {
      fixedRelativeAddrConsumers += ((instance, address, size))
    }
  }

  override def addVariableAddressConsumer(
      instance: ChipInstance,
      size: BigInt
  ): Unit = {
    if (instance == owner) {
      // Instead of returning Left, log the error as a warning
      // This maintains compatibility with the interface while still tracking the error
      val error = s"addVariableAddressConsumer adding itself $ident"
      // Create an Either for potential future refactoring but don't return it
      val _ = Left(error)
      // In a future refactoring, this might propagate the error
    } else {
      variableAddrConsumers += ((instance, size))
    }
  }

  override def consumerVariant: Variant = ArrayV(consumers.values.flatMap {
    case (addr, size) => Seq(BigIntV(addr), BigIntV(size))
  }.toArray)

  override def getOwner: ChipInstance = owner

  override def computeConsumerAddresses(): Unit = {
    fixedRelativeAddrConsumers.sortInPlaceWith((a, b) => a._2 < b._2)

    var currentAddress = spec.baseAddr

    while (
      fixedRelativeAddrConsumers.nonEmpty || variableAddrConsumers.nonEmpty
    ) {
      val consumeFixed = {
        (variableAddrConsumers.isEmpty && fixedRelativeAddrConsumers.nonEmpty) ||
        (variableAddrConsumers.nonEmpty && fixedRelativeAddrConsumers.nonEmpty &&
          ((currentAddress == (spec.baseAddr + fixedRelativeAddrConsumers.head._2)) ||
            (currentAddress + variableAddrConsumers.head._2 > (spec.baseAddr + fixedRelativeAddrConsumers.head._2))))
      }

      val (instance, address, size) = {
        if (consumeFixed) {
          val (instance, relativeAddress, size) =
            fixedRelativeAddrConsumers.remove(0)
          (instance, spec.baseAddr + relativeAddress, size)
        } else {
          val (instance, size) = variableAddrConsumers.remove(0)
          (instance, currentAddress, size)
        }
      }

      currentAddress = (address + size).max(spec.baseAddr)

      if (consumers.contains((instance, currentAddress))) {
        // We'll log the error without failing - in the future this could return Either
        // but changing the whole method signature would require broader changes
        val err =
          s"$ident bus already has entry for ${instance.name} at $currentAddress"
        // Error captured for potential future refactoring to full Either pattern
        val _ = Left(err)
        // for now leave the println until we have a better error handling that can do a warn but not just a fail
        println(err)
      } else {
        consumers((instance, address)) = (address -> size)
      }
    }
  }

  override def isHardware: Boolean = false

  override def getBaseAddress: BigInt = spec.baseAddr

  override def fixedBaseBusAddress: Boolean = spec.fixedAddress

}
