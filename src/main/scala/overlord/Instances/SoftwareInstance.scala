package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.{DefinitionTrait}

case class SoftwareInstance(ident: String,
                            override val definition: DefinitionTrait,
                          ) extends InstanceTrait {
}

object SoftwareInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ) : Option[SoftwareInstance] = {

		val sw = SoftwareInstance(ident, definition)
		sw.mergeAllAttributes(attribs)
		Some(sw)
	}
}