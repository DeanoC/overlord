package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.ChipDefinitionTrait

case class BridgeInstance(ident: String,
                          override val definition: ChipDefinitionTrait,
                         ) extends ChipInstance {

	lazy val addressWindowWidth: Int =
		Utils.lookupInt(attributes, "address_window_width", 16)

	override def copyMutate[A <: ChipInstance](nid: String): BridgeInstance =
		copy(ident = nid)

}

object BridgeInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[BridgeInstance] = {
		val bridge = BridgeInstance(ident, definition)
		bridge.mergeAllAttributes(attribs)
		Some(bridge)
	}
}