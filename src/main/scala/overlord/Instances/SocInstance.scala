package overlord.Instances

import ikuy_utils.Variant
import overlord.ChipDefinitionTrait

case class SocInstance(name: String,
                       definition: ChipDefinitionTrait
                      ) extends ChipInstance {
	override def isVisibleToSoftware: Boolean = true
}

object SocInstance {
	def apply(ident: String, definition: ChipDefinitionTrait, attribs: Map[String, Variant]): Option[SocInstance] = {
		val chip = SocInstance(ident, definition)
		chip.mergeAllAttributes(attribs)
		Some(chip)
	}
}