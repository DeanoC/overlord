package overlord.Instances

import ikuy_utils.Variant
import overlord.ChipDefinitionTrait

case class StorageInstance(ident: String,
                           private val defi: ChipDefinitionTrait
                          ) extends ChipInstance {
	override def definition:ChipDefinitionTrait = defi
	override def copyMutate[A <: ChipInstance](nid: String) : StorageInstance =
		copy(ident = nid)

}

object StorageInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[StorageInstance] = {
		val storage  = StorageInstance(ident, definition)
		storage.mergeAllAttributes(attribs)
		Some(storage)
	}
}