package overlord.Instances

import ikuy_utils.Variant
import overlord.ChipDefinitionTrait

case class IoInstance(ident: String,
                      override val definition: ChipDefinitionTrait,
                     ) extends ChipInstance {
	override def copyMutate[A <: ChipInstance](nid: String): IoInstance =
		copy(ident = nid)
}

object IoInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[IoInstance] = {
		val net = IoInstance(ident, definition)
		net.mergeAllAttributes(attribs)
		Some(net)

	}
}