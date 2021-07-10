package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.Definitions.DefinitionTrait
import toml.Value

case class CpuInstance(ident: String,
                       primary_boot: Boolean,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): CpuInstance =
		copy(ident = nid)

	lazy val width : Int =
		Utils.lookupInt(definition.attributes, key = "width", or = 32)
	lazy val triple: String =
		Utils.lookupString(definition.attributes, key = "triple", or = "unknown-unknown-unknown")
	lazy val sanitizedTriple: String = triple.replace("""-""", "")

}

object CpuInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[CpuInstance] = {

		val primary_boot = Utils.lookupBoolean(attribs,
		                                       key = "primary_boot",
		                                       or = false)

		Some(CpuInstance(ident, primary_boot, definition))
	}
}