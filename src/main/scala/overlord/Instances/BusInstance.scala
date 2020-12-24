package overlord.Instances

import ikuy_utils._
import overlord.Definitions.DefinitionTrait

case class BusInstance(ident: String,
                       supplierPrefixes: Seq[String],
                       consumerPrefix: String,
                       private val localParams: Map[String, Variant],
                       private val defi: DefinitionTrait
                      ) extends Instance {

	var connectedCount: Int = 0

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
		val iParams          = Map[String, Variant](
			("bus_data_width" -> attribs.getOrElse("bus_data_width", IntV(32))),
			("bus_address_width" ->
			 attribs.getOrElse("bus_address_width", IntV(32))),
			("bus_base" -> attribs.getOrElse("bus_base", BigIntV(0))),
			("bus_bank_size" -> attribs.getOrElse("bus_bank_size", BigIntV(1024)))
			)
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