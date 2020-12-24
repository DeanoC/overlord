package overlord.Instances

import ikuy_utils.Variant
import overlord.Definitions.DefinitionTrait
import toml.Value

case class CpuInstance(ident: String,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): CpuInstance =
		copy(ident = nid)

}

object CpuInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[CpuInstance] = {
		Some(CpuInstance(ident, definition))
	}
}