package overlord.Instances

import ikuy_utils._
import overlord.Definitions.DefinitionTrait

import scala.collection.mutable

case class BusInstance(ident: String,
                       private val defi: DefinitionTrait
                      ) extends Instance {

	lazy val busDataWidth    : Int         =
		Utils.lookupInt(attributes, "bus_data_width", 32)
	lazy val busAddrWidth    : Int         =
		Utils.lookupInt(attributes, "bus_address_width", 32)
	lazy val busBaseAddr     : BigInt      =
		Utils.lookupBigInt(attributes, "bus_base_address", 0)
	lazy val busBankAlignment: BigInt      =
		Utils.lookupBigInt(attributes, "bus_bank_alignment", 1024)
	lazy val supplierPrefixes: Seq[String] =
		Utils.lookupStrings(attributes, "supplier_prefix", "supplier_")
	lazy val consumerPrefix  : String      =
		Utils.lookupString(attributes, "consumer_prefix", "consumer${index}_")

	private val fixedAddrConsumers    = mutable.ArrayBuffer[(Instance, BigInt, BigInt)]()
	private val variableAddrConsumers = mutable.ArrayBuffer[(Instance, BigInt)]()

	def addFixedAddressConsumer(instance: Instance, address: BigInt, size: BigInt): Unit =
		fixedAddrConsumers += ((instance, address, size))

	def addVariableAddressConsumer(instance: Instance, size: BigInt): Unit =
		variableAddrConsumers += ((instance, size))

	def computeConsumerAddresses(): Unit = {
		fixedAddrConsumers.sortInPlaceWith((a, b) => a._2 < b._2)

		var currentAddress = busBaseAddr
		if (fixedAddrConsumers.nonEmpty && fixedAddrConsumers.head._2 < currentAddress) {
			println(f"${fixedAddrConsumers.head._1.ident} has invalid fixed addresss%n")
		}
		while (fixedAddrConsumers.nonEmpty || variableAddrConsumers.nonEmpty) {
			val consumeFixed = {
				(variableAddrConsumers.isEmpty && fixedAddrConsumers.nonEmpty) ||
				(
					variableAddrConsumers.nonEmpty && fixedAddrConsumers.nonEmpty &&
					((currentAddress == fixedAddrConsumers.head._2) ||
					 (currentAddress + variableAddrConsumers.head._2 > fixedAddrConsumers.head._2))
					)
			}

			val (instance, address, size) =
				if (consumeFixed)
					fixedAddrConsumers.remove(0)
				else {
					val (instance, size) = variableAddrConsumers.remove(0)
					(instance, currentAddress, size)
				}
			consumerIndices += (instance -> consumers.length)
			consumers += (address -> size)
			currentAddress = (address + size).max(busBaseAddr)
		}
	}

	def getIndex(instance: Instance): Int = {
		if (consumerInstances.contains(instance))
			consumerIndices(instance)
		else
			-1
	}

	def consumerCount: Int = consumers.length

	def consumersVariant: Variant = ArrayV(consumers.flatMap {
		case (addr, size) => Seq(BigIntV(addr), BigIntV(size))
	}.toArray)

	def getConsumerAddressAndSize(instance: Instance): (BigInt, BigInt) = {
		val index = getIndex(instance)
		if (index == -1) return (0, 0)
		consumers(index)
	}

	def consumerInstances: Seq[Instance] = consumerIndices.keysIterator.toSeq

	private var consumers       = mutable.ArrayBuffer[(BigInt, BigInt)]()
	private var consumerIndices = mutable.HashMap[Instance, Int]()

	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): BusInstance =
		copy(ident = nid)

	override val shared: Boolean = true
}

object BusInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[BusInstance] = {

		val bus = BusInstance(ident, definition)

		bus.mergeParameter(attribs, "bus_data_width")
		bus.mergeParameter(attribs, "bus_address_width")
		bus.mergeParameter(attribs, "bus_base_address")
		bus.mergeParameter(attribs, "bus_bank_alignment")
		bus.mergeParameter(attribs, "supplier_prefix")
		bus.mergeParameter(attribs, "consumer_prefix")


		Some(bus)
	}
}