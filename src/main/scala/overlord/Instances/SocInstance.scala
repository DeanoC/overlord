package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class SocInstance(ident: String,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): SocInstance =
		copy(ident = nid)

}

object SocInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[SocInstance] = {
		Some(SocInstance(ident, definition))
	}
}