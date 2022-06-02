package overlord.Instances

import ikuy_utils.Utils.VariantTable
import ikuy_utils.{ArrayV, BigIntV, Utils, Variant}
import overlord.Interfaces.SupplierBusLike

import scala.collection.mutable
import scala.reflect.ClassTag

case class BusSpec(name: String,
                   supplier: Boolean,
                   baseAddr: BigInt,
                   dataWidth: BigInt,
                   addrWidth: BigInt,
                   protocol: String,
                   prefix: String)

case class Bus(owner: ChipInstance, ident: String, attributes: VariantTable, spec: BusSpec) extends SupplierBusLike {
	private val fixedRelativeAddrConsumers = mutable.ArrayBuffer[(ChipInstance, BigInt, BigInt)]()
	private val variableAddrConsumers      = mutable.ArrayBuffer[(ChipInstance, BigInt)]()

	private var consumers = mutable.HashMap[ChipInstance, (BigInt, BigInt)]()

	def consumerInstances: Seq[ChipInstance] = consumers.keys.toSeq

	def consumerCount: Int = consumers.size

	// query interface delegated to owner
	override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = owner.getInterface[T]

	// bus like interface
	override def isSupplier: Boolean = spec.supplier

	override def getPrefix: String = spec.prefix

	override def getBusAlignment: BigInt = Utils.lookupBigInt(attributes.toMap, "bus_bank_alignment", 1024)

	override def addFixedRelativeAddressConsumer(instance: ChipInstance, address: BigInt, size: BigInt): Unit =
		if (instance == owner) println(s"addFixedAddressConsumer adding itself $ident")
		else fixedRelativeAddrConsumers += ((instance, address, size))

	override def addVariableAddressConsumer(instance: ChipInstance, size: BigInt): Unit =
		if (instance == owner) println(s"addVariableAddressConsumer adding itself $ident")
		else variableAddrConsumers += ((instance, size))

	override def consumerVariant: Variant = ArrayV(consumers.values.flatMap { case (addr, size) => Seq(BigIntV(addr), BigIntV(size)) }.toArray)

	override def getConsumerAddressAndSize(instance: ChipInstance): (BigInt, BigInt) = {
		if (consumers.contains(instance)) consumers(instance)
		else {
			println(s"${getOwner.name} doesn't have a consumer address for ${instance.name}")
			(-1, 0)
		}
	}

	override def getOwner: ChipInstance = owner

	override def computeConsumerAddresses(): Unit = {
		fixedRelativeAddrConsumers.sortInPlaceWith((a, b) => a._2 < b._2)

		var currentAddress = spec.baseAddr

		while (fixedRelativeAddrConsumers.nonEmpty || variableAddrConsumers.nonEmpty) {
			val consumeFixed = {
				(variableAddrConsumers.isEmpty && fixedRelativeAddrConsumers.nonEmpty) ||
				(variableAddrConsumers.nonEmpty && fixedRelativeAddrConsumers.nonEmpty &&
				 ((currentAddress == (spec.baseAddr + fixedRelativeAddrConsumers.head._2)) ||
				  (currentAddress + variableAddrConsumers.head._2 > (spec.baseAddr + fixedRelativeAddrConsumers.head._2))))
			}


			val (instance, address, size) = {
				if (consumeFixed) {
					val (instance, relativeAddress, size) = fixedRelativeAddrConsumers.remove(0)
					(instance, spec.baseAddr + relativeAddress, size)
				}
				else {
					val (instance, size) = variableAddrConsumers.remove(0)
					(instance, currentAddress, size)
				}
			}

			if (consumers.contains(instance)) {
				println(s"$ident bus already has entry for ${instance.name}")
			} else {
				consumers(instance) = (address -> size)
				currentAddress = (address + size).max(spec.baseAddr)
			}
		}
	}

	override def isHardware: Boolean = false

	override def getBaseAddress: BigInt = spec.baseAddr
}

