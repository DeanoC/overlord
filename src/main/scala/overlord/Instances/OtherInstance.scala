package overlord.Instances

import ikuy_utils.Variant
import overlord.ChipDefinitionTrait

case class OtherInstance(ident: String,
                         definition: ChipDefinitionTrait,
                        ) extends ChipInstance {
	override def copyMutate[A <: ChipInstance](nid: String): OtherInstance =
		copy(ident = nid)

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