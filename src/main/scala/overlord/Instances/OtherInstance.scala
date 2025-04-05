package overlord.Instances

import gagameos.Variant
import overlord.ChipDefinitionTrait

case class OtherInstance(name: String,
                         definition: ChipDefinitionTrait,
                        ) extends ChipInstance {
	override def isVisibleToSoftware: Boolean = true

}

object OtherInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[OtherInstance] = {
		val other = OtherInstance(ident, definition)
		other.mergeAllAttributes(attribs)
		Some(other)
	}
}