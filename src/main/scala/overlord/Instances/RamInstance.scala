package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class RamInstance(ident: String,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): RamInstance =
		copy(ident = nid)

}
object RamInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[RamInstance] = {
		Some(RamInstance(ident, definition))
	}
}