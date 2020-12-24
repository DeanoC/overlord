package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class OtherInstance(ident: String,
                         private val defi: DefinitionTrait
                        ) extends Instance {
	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): OtherInstance =
		copy(ident = nid)

}

object OtherInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[OtherInstance] = {
		Some(OtherInstance(ident, definition))
	}
}