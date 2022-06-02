package overlord.Instances

import ikuy_utils.Utils
import overlord.Chip.{Port, RegisterBank, Registers}
import overlord.Interfaces._
import overlord.{ChipDefinitionTrait, GatewareDefinitionTrait, HardwareDefinitionTrait}

import scala.collection.mutable
import scala.reflect.ClassTag

trait ChipInstance extends InstanceTrait with PortsLike with RegisterBankLike with MultiBusLike {
	override def definition: ChipDefinitionTrait

	lazy         val instanceNumber       : Int                               = Utils.lookupInt(attributes, "instance", 0)
	lazy         val ports                : mutable.HashMap[String, Port]     = mutable.HashMap[String, Port](definition.ports.toSeq: _*)
	lazy         val instanceRegisterBanks: mutable.ArrayBuffer[RegisterBank] = {
		if (attributes.contains("registers")) {
			Registers(this, Utils.toArray(attributes("registers"))).to(mutable.ArrayBuffer)
		} else mutable.ArrayBuffer()
	}
	private lazy val hasRegisters         : Boolean                           = registerBanks.nonEmpty
	private lazy val registerBanks        : Seq[RegisterBank]                 = {
		(instanceRegisterBanks ++
		 (if (definition.registers.nonEmpty) definition.registers else Seq())
			).toSeq
	}
	val instanceParameterKeys: mutable.HashSet[String] = mutable.HashSet()
	val splitIdent           : Array[String]           = name.split('.')
	private val busSpecs: Seq[BusSpec] = {
		if (!attributes.contains("buses")) Seq()
		else Utils.toArray(attributes("buses")).map(
			b => {
				val table = Utils.toTable(b)
				BusSpec(name = Utils.lookupString(table, "name", "NO_NAME"),
				        supplier = Utils.lookupBoolean(table, "supplier", true),
				        protocol = Utils.lookupString(table, "protocol", "internal"),
				        prefix = Utils.lookupString(table, "prefix", "internal"),
				        baseAddr = Utils.lookupBigInt(table, "base_address", 0),
				        dataWidth = Utils.lookupBigInt(table, "data_width", 32),
				        addrWidth = Utils.lookupBigInt(table, "address_width", 32))
			})
	}
	private val buses   : Seq[Bus]     = busSpecs.map(Bus(this, name, definition.attributes, _))

	def isGateware: Boolean = definition.isInstanceOf[GatewareDefinitionTrait]

	def isHardware: Boolean = definition.isInstanceOf[HardwareDefinitionTrait]

	def mergeParameterKey(key: String): Unit = instanceParameterKeys += key

	def isVisibleToSoftware: Boolean = hasRegisters || Utils.lookupBoolean(attributes, "visable_to_software", or = false)

	def mergePort(key: String, port: Port): Unit = ports.updateWith(key) {
		case Some(_) => Some(port)
		case None    => Some(port)
	}

	lazy val parameterKeys: Set[String] = instanceParameterKeys.toSet

	def getPort(lastName: String): Option[Port] =
		if (ports.contains(lastName)) Some(ports(lastName)) else None

	def getMatchNameAndPort(nameToMatch: String): (Option[String], Option[Port]) = {
		val nameWithoutBits = nameToMatch.split('[').head

		def WildCardMatch(nameId: Array[String], instanceId: Array[String]) = {
			// wildcard match
			val is = for ((id, i) <- instanceId.zipWithIndex) yield
				if ((i < nameId.length) && (id == "_" || id == "*"))
					nameId(i)
				else id

			val ms = for ((id, i) <- nameId.zipWithIndex) yield
				if ((i < is.length) && (id == "_" || id == "*")) is(i)
				else id

			if (ms sameElements is)
				(Some(ms.mkString(".")), getPort(ms(0)))
			else {
				val msv = ms.reverse
				val mp  = msv.head
				val tms = if (msv.length > 1) msv.tail.reverse else msv

				val port = getPort(mp)
				if ((is sameElements tms) && port.nonEmpty)
					(Some(s"${ms.mkString(".")}"), port)
				else (None, None)
			}
		}

		if (nameToMatch == name)
			(Some(nameToMatch), getPort(nameToMatch.split('.').last))
		else if (nameWithoutBits == name)
			(Some(nameWithoutBits), getPort(nameWithoutBits.split('.').last))
		else {
			val match0 = WildCardMatch(nameWithoutBits.split('.'), name.split('.'))
			if (match0._1.isDefined) return match0
			WildCardMatch(nameWithoutBits.split('.'), definition.defType.ident.toArray)
		}
	}

	override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
		val PortsLike_        = classOf[PortsLike]
		val RegisterBankLike_ = classOf[RegisterBankLike]
		val MultiBusLike_     = classOf[MultiBusLike]

		tag.runtimeClass match {
			case MultiBusLike_     => Some(asInstanceOf[T])
			case RegisterBankLike_ => Some(asInstanceOf[T])
			case PortsLike_        => Some(asInstanceOf[T])
			case _                 => super.getInterface[T]
		}
	}

	// ports like interface
	override def getOwner: ChipInstance = this

	override def getPortsStartingWith(startsWith: String): Seq[Port] = ports.filter(_._1.startsWith(startsWith)).values.toSeq

	override def getPortsMatchingName(name: String): Seq[Port] = ports.filter(_._1 == name).values.toSeq

	// register bank like
	override def maxInstances: Int = definition.maxInstances

	override def banks: Seq[RegisterBank] = registerBanks

	override def generateInstancedRegisterBank(): Unit = {}

	override def getBus(index: Int): Option[BusLike] = if (index < numberOfBuses) Some(buses(index)) else None

	override def numberOfBuses: Int = buses.length

	override def getFirstSupplierBusByName(name: String): Option[SupplierBusLike] = buses.find(b => nameMatch(b.spec, name) && b.spec.supplier)

	// multi bus like interface
	private def nameMatch(busSpec: BusSpec, name: String): Boolean =
		(name == busSpec.name ||
		 name.replace("${index}", "") == busSpec.name ||
		 name == busSpec.name.replace("${index}", "") ||
		 name.replace("${index}", "") == busSpec.name.replace("${index}", ""))

	override def getFirstSupplierBusOfProtocol(protocol: String): Option[SupplierBusLike] = buses.find(b => protocolMatch(b.spec, protocol) && b.spec.supplier)

	private def protocolMatch(busSpec: BusSpec, protocol: String): Boolean =
		(protocol == busSpec.protocol ||
		 protocol.replace("${index}", instanceNumber.toString) == busSpec.protocol ||
		 protocol == busSpec.protocol.replace("${index}", instanceNumber.toString) ||
		 protocol.replace("${index}", instanceNumber.toString) == busSpec.protocol.replace("${index}", instanceNumber.toString))

	override def getFirstConsumerBusByName(name: String): Option[BusLike] = buses.find(b => nameMatch(b.spec, name) && !b.spec.supplier)

	override def getFirstConsumerBusOfProtocol(protocol: String): Option[BusLike] = buses.find(b => protocolMatch(b.spec, protocol) && !b.spec.supplier)

}