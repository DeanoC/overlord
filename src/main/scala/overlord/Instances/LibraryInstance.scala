package overlord.Instances

import ikuy_utils.Variant
import overlord.SoftwareDefinitionTrait

case class LibraryInstance(ident: String,
                           sharedWithHost: Boolean,
                           hostOnly: Boolean,
                           override val definition: SoftwareDefinitionTrait,
                          ) extends SoftwareInstance {
	override val folder =
		if (sharedWithHost) "libs" else if (hostOnly) "libs_host" else "libs_target"
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

		val sharedWithHost = attribs.contains("shared_with_host") |
		                     definition.attributes.contains("shared_with_host")
		val hostOnly       = attribs.contains("host_only") |
		                     definition.attributes.contains("host_only")

		val sw = LibraryInstance(ident, sharedWithHost, hostOnly, definition)
		sw.mergeAllAttributes(attribs)
		Some(sw)
	}
}