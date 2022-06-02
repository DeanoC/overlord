package overlord.Instances

import ikuy_utils._
import overlord.ChipDefinitionTrait
import overlord.Interfaces.{BusLike, MultiBusLike, SupplierBusLike}

import scala.reflect.ClassTag

case class BusInstance(name: String, override val definition: ChipDefinitionTrait) extends ChipInstance {
	private val busSpec: BusSpec = {
		val attrs = definition.attributes
		val table = attrs
		BusSpec(name = Utils.lookupString(table, "bus_name", "internal"),
		        supplier = Utils.lookupBoolean(table, "bus_supplier", true),
		        protocol = Utils.lookupString(table, "bus_protocol", "internal"),
		        prefix = Utils.lookupString(table, "bus_prefix", "internal"),
		        baseAddr = Utils.lookupBigInt(table, "base_address", 0),
		        dataWidth = Utils.lookupBigInt(table, "data_width", 32),
		        addrWidth = Utils.lookupBigInt(table, "address_width", 32))
	}
	private val bus    : Bus     = Bus(this, name, definition.attributes, busSpec)

	// query interface interface responder
	override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
		val MultiBusLike_ = classOf[MultiBusLike]
		val BusLike_      = classOf[BusLike]

		tag.runtimeClass match {
			case MultiBusLike_ => Some(MultiBusAdaptor(this).asInstanceOf[T])
			case BusLike_      => Some(bus.asInstanceOf[T])
			case _             => super.getInterface[T](tag)
		}
	}

	// adapter to make this single bug work with multi bus code
	private case class MultiBusAdaptor(parent: BusInstance) extends MultiBusLike {
		override def getBus(index: Int): Option[BusLike] = {
			if (index >= numberOfBuses) None
			else Some(bus.asInstanceOf[BusLike])
		}

		override def numberOfBuses: Int = 1

		override def getFirstSupplierBusByName(name: String): Option[SupplierBusLike] =
			Some(getFirstBusByName(name, supplier = true).getOrElse(return None).asInstanceOf[SupplierBusLike])

		private def getFirstBusByName(name: String, supplier: Boolean): Option[BusLike] = {
			if (name.isEmpty) None
			else if (busSpec.supplier != supplier) {
				println(s"${parent.name} ${busSpec.name} bus is in the wrong direction")
				None
			} else if (nameMatch(name)) {
				println(s"${parent.name} does not support bus named ${busSpec.name}")
				None
			} else Some(bus)
		}

		private def nameMatch(name: String): Boolean =
			(name == busSpec.name ||
			 name.replace("${index}", instanceNumber.toString) == busSpec.name ||
			 name == busSpec.name.replace("${index}", instanceNumber.toString) ||
			 name.replace("${index}", instanceNumber.toString) == busSpec.name.replace("${index}", instanceNumber.toString))

		override def getFirstSupplierBusOfProtocol(protocol: String): Option[SupplierBusLike] =
			Some(getFirstBusOfProtocol(protocol, supplier = true).getOrElse(return None).asInstanceOf[SupplierBusLike])

		private def getFirstBusOfProtocol(protocol: String, supplier: Boolean): Option[BusLike] = {
			if (busSpec.supplier != supplier) {
				println(s"${parent.name} ${busSpec.name} bus is in the wrong direction")
				None
			} else if (protocolMatch(protocol)) {
				println(s"${parent.name} does not support bus protocol ${busSpec.protocol}")
				None
			} else Some(bus)
		}

		private def protocolMatch(protocol: String): Boolean =
			(protocol == busSpec.protocol ||
			 protocol.replace("${index}", instanceNumber.toString) == busSpec.protocol ||
			 protocol == busSpec.protocol.replace("${index}", instanceNumber.toString) ||
			 protocol.replace("${index}", instanceNumber.toString) == busSpec.protocol.replace("${index}", instanceNumber.toString))

		override def getFirstConsumerBusByName(name: String): Option[BusLike] = getFirstBusByName(name, supplier = false)

		override def getFirstConsumerBusOfProtocol(protocol: String): Option[BusLike] = getFirstBusOfProtocol(protocol, supplier = false)

		override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = parent.getInterface[T]

		override def getOwner: ChipInstance = parent
	}
}

object BusInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[BusInstance] = {

		val bus = BusInstance(ident, definition)
		bus.mergeAllAttributes(attribs)
		Some(bus)
	}
}