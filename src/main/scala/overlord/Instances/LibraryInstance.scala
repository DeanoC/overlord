package overlord.Instances

import ikuy_utils.Variant
import overlord.SoftwareDefinitionTrait

case class LibraryInstance(override val name: String,
                           override val definition: SoftwareDefinitionTrait,
                          ) extends SoftwareInstance {
	override val folder = "libs"
}

object LibraryInstance {
	def apply(ident: String,
	          definition: SoftwareDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[LibraryInstance] = {

		if (!attribs.contains("name") &&
		    !definition.attributes.contains("name")) {
			println(f"Libraries must have a name attribute%n")
			return None
		}

		val sw = LibraryInstance(ident, definition)
		sw.mergeAllAttributes(attribs)
		Some(sw)
	}
}