package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.ChipDefinitionTrait
import toml.Value

case class CpuInstance(ident: String,
                       override val definition: ChipDefinitionTrait,
                      ) extends ChipInstance {
	lazy val primaryBoot: Boolean =
		Utils.lookupBoolean(attributes, "primary_boot", or = false)
	lazy val width : Int = Utils.lookupInt(attributes, "width", 32)
	lazy val triple: String =
		Utils.lookupString(attributes, key = "triple", or = "ERR-ERR-ERR")
	lazy val sanitizedTriple: String = triple.replace("-", "_")
	lazy val cpuCount : Int = Utils.lookupInt(attributes, "core_count", 1)

	override def copyMutate[A <: ChipInstance](nid: String): CpuInstance =
		copy(ident = nid)


}

object CpuInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[CpuInstance] = {

		val cpu  = CpuInstance(ident, definition)
		cpu.mergeAllAttributes(attribs)

		Some(cpu)
	}
}