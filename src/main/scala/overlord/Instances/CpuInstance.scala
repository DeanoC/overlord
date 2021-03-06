package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.Definitions.DefinitionTrait
import toml.Value

case class CpuInstance(ident: String,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	lazy val primaryBoot: Boolean =
		Utils.lookupBoolean(attributes, "primary_boot", or = false)
	lazy val width : Int = Utils.lookupInt(attributes, "width", 32)
	lazy val triple: String =
		Utils.lookupString(attributes, key = "triple", or = "unknown-unknown-unknown")

	lazy val sanitizedTriple: String = triple.replace("""-""", "")

	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): CpuInstance =
		copy(ident = nid)


}

object CpuInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[CpuInstance] = {

		val cpu  = CpuInstance(ident, definition)

		cpu.mergeParameter(attribs, "primary_boot")

		Some(cpu)
	}
}