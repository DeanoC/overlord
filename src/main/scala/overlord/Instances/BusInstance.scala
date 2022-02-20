package overlord.Instances

import ikuy_utils._
import overlord.ChipDefinitionTrait

import scala.collection.mutable

case class BusInstance(ident: String,
                       override val definition: ChipDefinitionTrait,
                      ) extends ChipInstance {

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

	private val fixedAddrConsumers    = mutable.ArrayBuffer[(ChipInstance, BigInt, BigInt)]()
	private val variableAddrConsumers = mutable.ArrayBuffer[(ChipInstance, BigInt)]()

	def addFixedAddressConsumer(instance: ChipInstance, address: BigInt, size: BigInt): Unit =
		fixedAddrConsumers += ((instance, address, size))

	def addVariableAddressConsumer(instance: ChipInstance, size: BigInt): Unit =
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

			val indexCount = getIndices(instance).length
			consumerIndices += ((instance,indexCount) -> consumers.length)
			consumers += (address -> size)
			currentAddress = (address + size).max(busBaseAddr)
		}
	}

	def getFirstIndex(instance: ChipInstance): Int = {
		if (consumerIndices.contains((instance,0)))
			consumerIndices((instance,0))
		else
			-1
	}
	def getIndices(instance: ChipInstance): Seq[Int] = {
		consumerIndices.filter(_._1._1 == instance).values.toSeq
	}

	def consumerCount: Int = consumers.length

	def consumersVariant: Variant = ArrayV(consumers.flatMap {
		case (addr, size) => Seq(BigIntV(addr), BigIntV(size))
	}.toArray)

	def getFirstConsumerAddressAndSize(instance: ChipInstance): (BigInt, BigInt) = {
		val index = getFirstIndex(instance)
		if (index == -1) return (0, 0)
		consumers(index)
	}
	def getConsumerAddressesAndSizes(instance: ChipInstance): Seq[(BigInt, BigInt)] = {
		getIndices(instance).map(consumers(_))
	}

	def consumerInstances: Seq[ChipInstance] =
		consumerIndices.keysIterator.map(_._1).distinct.toSeq

	private var consumers       = mutable.ArrayBuffer[(BigInt, BigInt)]()
	private var consumerIndices = mutable.HashMap[(ChipInstance, Int), Int]()

	override def copyMutate[A <: ChipInstance](nid: String): BusInstance =
		copy(ident = nid)
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