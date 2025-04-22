package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.hardware.{Port, RegisterBank, Registers}
import com.deanoc.overlord.{
  QueryInterface
}
import com.deanoc.overlord.definitions.{HardwareDefinition, GatewareDefinition}
import com.deanoc.overlord.interfaces.{
  PortsLike,
  RegisterBankLike,
  MultiBusLike,
  RamLike,
  SupplierBusLike
}
import com.deanoc.overlord.interfaces.BusLike
import com.deanoc.overlord.interfaces._

import scala.collection.mutable
import scala.reflect.ClassTag
import com.deanoc.overlord.definitions.GatewareDefinition

trait ChipInstance
    extends InstanceTrait
    with PortsLike
    with RegisterBankLike
    with MultiBusLike {
  override def definition: HardwareDefinition

  var moduleName: String = name
  lazy val instanceNumber: Int = Utils.lookupInt(attributes, "instance", 0)
  lazy val ports: mutable.HashMap[String, Port] =
    mutable.HashMap[String, Port](definition.ports.toSeq: _*)
  lazy val instanceRegisterBanks: mutable.ArrayBuffer[RegisterBank] = {
    if (attributes.contains("registers")) {
      // Use scala.collection.immutable.ArraySeq to avoid implicit array conversion
      import scala.collection.immutable.ArraySeq
      val registersVariant = attributes("registers")
      val registerArray =
        if (registersVariant.isInstanceOf[Array[_]]) {
          // If it's already an array, use ArraySeq.unsafeWrapArray
          ArraySeq.unsafeWrapArray(
            registersVariant.asInstanceOf[Array[Variant]]
          )
        } else {
          // Otherwise convert to a Seq and then to an ArrayBuffer
          Utils.toArray(registersVariant).toIndexedSeq
        }
      mutable.ArrayBuffer.from(Registers(this, registerArray))
    } else mutable.ArrayBuffer()
  }
  private lazy val hasRegisters: Boolean = registerBanks.nonEmpty
  private lazy val registerBanks: Seq[RegisterBank] = {
    (instanceRegisterBanks ++
      (if (definition.registers.nonEmpty) definition.registers
       else Seq())).toSeq
  }
  val instanceParameterKeys: mutable.HashSet[String] = mutable.HashSet()

  val splitIdent: Array[String] = name.split('.')
  private val busSpecs: Seq[BusSpec] = {
    if (!attributes.contains("buses")) Seq()
    else
      Utils
        .toArray(attributes("buses"))
        .toIndexedSeq
        .map(b => {
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
  private val buses: Seq[Bus] =
    busSpecs.map(Bus(this, name, definition.config.attributesAsVariant, _)) 

  def isGateware: Boolean = definition.isInstanceOf[GatewareDefinition]

  def isHardware: Boolean = definition.isInstanceOf[HardwareDefinition]

  def mergeParameterKey(key: String): Unit = instanceParameterKeys += key

  def isVisibleToSoftware: Boolean = hasRegisters || Utils.lookupBoolean(
    attributes,
    "visible_to_software",
    or = false
  )

  def mergePort(key: String, port: Port): Unit = ports.updateWith(key) {
    case Some(_) => Some(port)
    case None    => Some(port)
  }

  lazy val parameterKeys: Set[String] = instanceParameterKeys.toSet

  override def getPort(lastName: String): Option[Port] =
    if (ports.contains(lastName)) Some(ports(lastName)) else None

  override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
    val PortsLike_ = classOf[PortsLike]
    val RegisterBankLike_ = classOf[RegisterBankLike]
    val MultiBusLike_ = classOf[MultiBusLike]

    tag.runtimeClass match {
      case MultiBusLike_     => Some(asInstanceOf[T])
      case RegisterBankLike_ => Some(asInstanceOf[T])
      case PortsLike_        => Some(asInstanceOf[T])
      case _                 => super.getInterface[T]
    }
  }

  // ports like interface
  override def getOwner: ChipInstance = this

  override def getPortsStartingWith(startsWith: String): Seq[Port] =
    ports.filter(_._1.startsWith(startsWith)).values.toSeq

  override def getPortsMatchingName(name: String): Seq[Port] =
    ports.filter(_._1 == name).values.toSeq

  // register bank like
  override def maxInstances: Int = definition.maxInstances

  override def banks: Seq[RegisterBank] = registerBanks

  override def getBus(index: Int): Option[BusLike] =
    if (index < numberOfBuses) Some(buses(index)) else None

  override def numberOfBuses: Int = buses.length

  override def getFirstSupplierBusByName(
      name: String
  ): Option[SupplierBusLike] =
    buses.find(b => nameMatch(b.spec, name) && b.spec.supplier)

  // multi bus like interface
  private def nameMatch(busSpec: BusSpec, name: String): Boolean =
    (name == busSpec.name ||
      name.replace("${index}", "") == busSpec.name ||
      name == busSpec.name.replace("${index}", "") ||
      name.replace("${index}", "") == busSpec.name.replace("${index}", ""))

  override def getFirstSupplierBusOfProtocol(
      protocol: String
  ): Option[SupplierBusLike] =
    buses.find(b => protocolMatch(b.spec, protocol) && b.spec.supplier)

  private def protocolMatch(busSpec: BusSpec, protocol: String): Boolean =
    (protocol == busSpec.protocol ||
      protocol.replace(
        "${index}",
        instanceNumber.toString
      ) == busSpec.protocol ||
      protocol == busSpec.protocol.replace(
        "${index}",
        instanceNumber.toString
      ) ||
      protocol.replace("${index}", instanceNumber.toString) == busSpec.protocol
        .replace("${index}", instanceNumber.toString))

  override def getFirstConsumerBusByName(name: String): Option[BusLike] =
    buses.find(b => nameMatch(b.spec, name) && !b.spec.supplier)

  override def getFirstConsumerBusOfProtocol(
      protocol: String
  ): Option[BusLike] =
    buses.find(b => protocolMatch(b.spec, protocol) && !b.spec.supplier)

}
