package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class StorageInstance(ident: String,
                           private val defi: DefinitionTrait
                          ) extends Instance {
	override def definition:DefinitionTrait = defi
	override def copyMutate[A <: Instance](nid: String) : StorageInstance =
		copy(ident = nid)

}

object StorageInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[StorageInstance] = {
		Some(StorageInstance(ident, definition))
	}
}