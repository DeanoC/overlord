package overlord.Instances

import ikuy_utils.{BigIntV, IntV, Variant}
import overlord.Definitions.DefinitionTrait

case class BridgeInstance(ident: String,
                          private val localParams: Map[String, Variant],
                          private val defi: DefinitionTrait
                         ) extends Instance {

	lazy val addressWindowWidth: Int =
		localParams("address_window_width").asInstanceOf[BigIntV].value.toInt

	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): BridgeInstance =
		copy(ident = nid)

}

object BridgeInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[BridgeInstance] = {
		//@formatter:off
		val iParams = Map[String, Variant]( elems =
	      "address_window_width" -> attribs.getOrElse("address_window_width", BigIntV(16)),
	    )
		//@formatter:on
		Some(BridgeInstance(ident, iParams, definition))
	}
}