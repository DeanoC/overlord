package overlord.Instances

import gagameos.Variant
import overlord.ChipDefinitionTrait

case class IoInstance(name: String,
                      override val definition: ChipDefinitionTrait,
                     ) extends ChipInstance {
	override def isVisibleToSoftware: Boolean = true
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