package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.ChipDefinitionTrait
import overlord.Interfaces.{BusLike, MultiBusLike, SupplierBusLike}

import scala.reflect.ClassTag

case class CpuInstance(name: String,
                       override val definition: ChipDefinitionTrait,
                      ) extends ChipInstance with MultiBusLike {
	lazy val triple        : String = Utils.lookupString(attributes, key = "triple", or = "ERR-ERR-ERR")
	lazy val maxAtomicWidth: Int    = Utils.lookupInt(attributes, "max_atomic_width", 0)

	lazy    val width            : Int          = Utils.lookupInt(attributes, "width", 32)
	lazy    val maxBitOpTypeWidth: Int          = Utils.lookupInt(attributes, "max_bitop_type_width", 32)
	lazy    val sanitizedTriple  : String       = triple.replace("-", "_")
	lazy    val cpuCount         : Int          = Utils.lookupInt(attributes, "core_count", 1)
	lazy    val host             : Boolean      = definition.defType.ident.last == "host"
	lazy    val cpuType          : String       = if (host) "host" else definition.defType.ident(1) // cpu.$CpuType.blah.blash
	lazy    val gccFlags         : String       = Utils.lookupString(attributes, "gcc_flags", "")
	private val busSpecs         : Seq[BusSpec] = {
		val attrs = definition.attributes
		if (!attrs.contains("buses")) Seq()
		else Utils.toArray(attrs("buses")).map(
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
	private val buses            : Seq[Bus]     = busSpecs.map(Bus(this, name, definition.attributes, _))

	override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
		val MultiBusLike_ = classOf[MultiBusLike]

		tag.runtimeClass match {
			case MultiBusLike_ => Some(asInstanceOf[T])
			case _             => super.getInterface[T](tag)
		}
	}

	override def getBus(index: Int): Option[BusLike] = if (index < numberOfBuses) Some(buses(index)) else None

	// multi bus like interface
	override def numberOfBuses: Int = buses.length

	override def getFirstSupplierBusByName(name: String): Option[SupplierBusLike] = buses.find(b => b.spec.name == name && b.spec.supplier)

	override def getFirstSupplierBusOfProtocol(protocol: String): Option[SupplierBusLike] = buses.find(b => b.spec.protocol == protocol && b.spec.supplier)

	override def getFirstConsumerBusByName(name: String): Option[BusLike] = buses.find(b => b.spec.name == name && !b.spec.supplier)

	override def getFirstConsumerBusOfProtocol(protocol: String): Option[BusLike] = buses.find(b => b.spec.protocol == protocol && !b.spec.supplier)

}

object CpuInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[CpuInstance] = {

		val cpu = CpuInstance(ident, definition)
		cpu.mergeAllAttributes(attribs)

		Some(cpu)
	}
}