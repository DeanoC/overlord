package overlord.Interfaces

import ikuy_utils.Variant
import overlord.Chip.{Port, RegisterBank}
import overlord.Connections.{Connected, ConnectionDirection}
import overlord.Instances.ChipInstance

import scala.reflect.ClassTag

trait QueryInterface {
	def hasInterface[T](implicit tag: ClassTag[T]): Boolean = getInterface[T](tag).nonEmpty

	def getInterface[T](implicit tag: ClassTag[T]): Option[T] = None

	def getInterfaceUnwrapped[T](implicit tag: ClassTag[T]): T = getInterface[T](tag).get
}

trait ChipLike extends QueryInterface {
	def getOwner: ChipInstance
}

trait PortsLike extends ChipLike {
	def getPortsStartingWith(startsWith: String): Seq[Port]

	def getPortsMatchingName(name: String): Seq[Port]
}

trait BusLike extends ChipLike {
	def isHardware: Boolean

	def isSupplier: Boolean

	def getPrefix: String
}

trait SupplierBusLike extends BusLike {
	override def isSupplier: Boolean = true

	def addFixedRelativeAddressConsumer(instance: ChipInstance, address: BigInt, size: BigInt): Unit

	def addVariableAddressConsumer(instance: ChipInstance, size: BigInt): Unit

	def computeConsumerAddresses(): Unit

	def consumerVariant: Variant

	def getConsumerAddressAndSize(instance: ChipInstance): (BigInt, BigInt)

	def getBusAlignment: BigInt

	def getBaseAddress: BigInt
}


trait MultiBusLike extends ChipLike {
	def numberOfBuses: Int

	def getBus(index: Int): Option[BusLike]

	def getFirstSupplierBusByName(name: String): Option[SupplierBusLike]

	def getFirstSupplierBusOfProtocol(protocol: String): Option[SupplierBusLike]

	def getFirstConsumerBusByName(name: String): Option[BusLike]

	def getFirstConsumerBusOfProtocol(protocol: String): Option[BusLike]

}

trait RamLike extends ChipLike {
	// sequence of start and size in bytes
	def getRanges: Seq[(BigInt, BigInt)]
}

trait UnConnectedLike extends QueryInterface {
	def firstFullName: String

	def direction: ConnectionDirection

	def secondFullName: String

	def preConnect(unexpanded: Seq[ChipInstance]): Unit

	def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit

	def connect(unexpanded: Seq[ChipInstance]): Seq[Connected]
}

trait UnconnectedPortsLike extends QueryInterface {

}

trait BusConnectionLike extends QueryInterface {
}

trait RegisterBankLike extends QueryInterface {
	def maxInstances: Int

	def banks: Seq[RegisterBank]

	def generateInstancedRegisterBank(): Unit
}