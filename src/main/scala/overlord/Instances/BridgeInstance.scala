package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class BridgeInstance(ident: String,
                          private val defi: DefinitionTrait
                         ) extends Instance {

	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): BridgeInstance =
		copy(ident = nid)

}

object BridgeInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[BridgeInstance] = {
		Some(BridgeInstance(ident, definition))
	}
}