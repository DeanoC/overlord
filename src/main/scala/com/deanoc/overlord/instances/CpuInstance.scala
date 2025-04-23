package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.definitions.HardwareDefinition
import com.deanoc.overlord.config.CpuDefinitionConfig

import com.deanoc.overlord.interfaces.{MultiBusLike, SupplierBusLike}
import com.deanoc.overlord.interfaces.BusLike

import scala.reflect.ClassTag
import com.deanoc.overlord.utils.Logging

case class CpuInstance(
    name: String,
    override val definition: HardwareDefinition
) extends HardwareInstance
    with MultiBusLike {

  lazy val cpuConfig = definition.config.asInstanceOf[CpuDefinitionConfig]
  lazy val triple: String = cpuConfig.triple
  lazy val maxAtomicWidth: Int = cpuConfig.max_atomic_width
  lazy val width: Int = cpuConfig.width
  lazy val maxBitOpTypeWidth: Int = cpuConfig.max_bitop_type_width
  lazy val sanitizedTriple: String = triple.replace("-", "_")
  lazy val cpuCount: Int = cpuConfig.core_count
  lazy val host: Boolean = definition.defType.ident.last == "host"
  lazy val cpuType: String = if (host) "host" else definition.defType.ident(1)
  lazy val gccFlags: String = cpuConfig.gcc_flags

  private val busSpecs: Seq[BusSpec] = {
    val attrs = definition.config.attributesAsVariant
    if (!attrs.contains("buses")) Seq()
    else {
      // Using explicit toIndexedSeq instead of implicit conversion
      val busesArray = Utils.toArray(attrs("buses"))
      busesArray.toIndexedSeq.map(b => {
        val table = Utils.toTable(b)
        BusSpec(
          name = Utils.lookupString(table, "name", "NO_NAME"),
          supplier = Utils.lookupBoolean(table, "supplier", true),
          protocol = Utils.lookupString(table, "protocol", "internal"),
          prefix = Utils.lookupString(table, "prefix", "internal"),
          baseAddr = Utils.lookupBigInt(table, "base_address", 0),
          dataWidth = Utils.lookupBigInt(table, "data_width", 32),
          addrWidth = Utils.lookupBigInt(table, "address_width", 32),
          fixedAddress =
            Utils.lookupBoolean(table, "fixed_base_address", or = false)
        )
      })
    }
  }
  private val buses: Seq[Bus] =
    busSpecs.map(Bus(this, name, definition.config.attributesAsVariant, _))

  override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
    val MultiBusLike_ = classOf[MultiBusLike]

    tag.runtimeClass match {
      case MultiBusLike_ => Some(asInstanceOf[T])
      case _             => super.getInterface[T](tag)
    }
  }

  override def getBus(index: Int): Option[BusLike] =
    if (index < numberOfBuses) Some(buses(index)) else None

  // multi bus like interface
  override def numberOfBuses: Int = buses.length

  override def getFirstSupplierBusByName(
      name: String
  ): Option[SupplierBusLike] =
    buses.find(b => b.spec.name == name && b.spec.supplier)

  override def getFirstSupplierBusOfProtocol(
      protocol: String
  ): Option[SupplierBusLike] =
    buses.find(b => b.spec.protocol == protocol && b.spec.supplier)

  override def getFirstConsumerBusByName(name: String): Option[BusLike] =
    buses.find(b => b.spec.name == name && !b.spec.supplier)

  override def getFirstConsumerBusOfProtocol(
      protocol: String
  ): Option[BusLike] =
    buses.find(b => b.spec.protocol == protocol && !b.spec.supplier)

}
