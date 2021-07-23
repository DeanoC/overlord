package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class NetInstance(ident: String,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): NetInstance =
		copy(ident = nid)
}

object NetInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[NetInstance] = {
		Some(NetInstance(ident, definition))
	}
}