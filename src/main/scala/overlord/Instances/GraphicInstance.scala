package overlord.Instances

import gagameos.Variant
import overlord.ChipDefinitionTrait

case class GraphicInstance(name: String,
                           private val defi: ChipDefinitionTrait
                          ) extends ChipInstance {
	override def definition: ChipDefinitionTrait = defi

	override def isVisibleToSoftware: Boolean = true

}

object GraphicInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[StorageInstance] = {
		val storage = StorageInstance(ident, definition)
		storage.mergeAllAttributes(attribs)
		Some(storage)
	}
}