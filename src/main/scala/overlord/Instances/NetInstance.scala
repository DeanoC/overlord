package overlord.Instances

import ikuy_utils.Variant
import overlord.ChipDefinitionTrait
import toml.Value

case class NetInstance(ident: String,
                       override val definition: ChipDefinitionTrait,
                      ) extends ChipInstance {
	override def copyMutate[A <: ChipInstance](nid: String): NetInstance =
		copy(ident = nid)
}

object NetInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[NetInstance] = {
		val net = NetInstance(ident, definition)
		net.mergeAllAttributes(attribs)
		Some(net)

	}
}