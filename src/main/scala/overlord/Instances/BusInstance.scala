package overlord.Instances

import ikuy_utils._
import overlord.Definitions.DefinitionTrait

import scala.collection.mutable

case class BusInstance(ident: String,
                       supplierPrefixes: Seq[String],
                       consumerPrefix: String,
                       private val localParams: Map[String, Variant],
                       private val defi: DefinitionTrait
                      ) extends Instance {

	def addConsumer(instance: Instance, address: BigInt, size: BigInt): Unit = {
		consumerIndices += (instance -> consumers.length)
		consumers += (address -> size)
	}

	def getIndex(instance: Instance): Int = {
		if( consumerInstances.contains(instance))
			consumerIndices(instance)
		else
			-1
	}

	def consumerCount: Int = consumers.length

	def consumersVariant: Variant = ArrayV(consumers.flatMap{
		case (addr, size) => Seq(BigIntV(addr), BigIntV(size))
	}.toArray)

	def getConsumerAddressAndSize(instance: Instance) : (BigInt, BigInt) = {
		val index = getIndex(instance)
		if(index == -1) return(0,0)
		consumers(index)
	}

	def consumerInstances: Seq[Instance] = consumerIndices.keysIterator.toSeq

	private var consumers = mutable.ArrayBuffer[(BigInt, BigInt)]()
	private var consumerIndices = mutable.HashMap[Instance, Int]()

	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): BusInstance =
		copy(ident = nid)

	override def parameters: Map[String, Variant] =
		super.parameters ++ localParams

	override val shared: Boolean = true
}

object BusInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[BusInstance] = {

		//@formatter:off
		val iParams = Map[String, Variant]( elems =
			"bus_data_width" -> attribs.getOrElse("bus_data_width", IntV(32)) ,
			"bus_address_width" -> attribs.getOrElse("bus_address_width", IntV(32)) ,
			"bus_base_address" -> attribs.getOrElse("bus_base_address", BigIntV(0)) ,
			"bus_bank_size" -> attribs.getOrElse("bus_bank_size", BigIntV(1024))
			)
		//@formatter:on

		val supplierPrefixes = Utils.lookupStrings(definition.attributes,
		                                           "supplier_prefix",
		                                           "supplier_")

		val consumerPrefix = Utils.lookupString(definition.attributes,
		                                        "consumer_prefix",
		                                        "consumers_${index}_")

		Some(BusInstance(ident,
		                 supplierPrefixes,
		                 consumerPrefix,
		                 iParams,
		                 definition))
	}
}