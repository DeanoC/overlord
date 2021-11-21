package overlord.Instances

import ikuy_utils.Variant
import overlord.SoftwareDefinitionTrait

case class ProgramInstance(ident: String,
                           override val definition: SoftwareDefinitionTrait,
                          ) extends SoftwareInstance {
	override val folder = "programs"
}

object ProgramInstance {
	def apply(ident: String,
	          definition: SoftwareDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[ProgramInstance] = {

		if (!attribs.contains("name") &&
		    !definition.attributes.contains("name")) {
			println(f"Programs must have a name attribute%n")
			return None
		}

		val sw = ProgramInstance(ident, definition)
		sw.mergeAllAttributes(attribs)
		Some(sw)
	}
}