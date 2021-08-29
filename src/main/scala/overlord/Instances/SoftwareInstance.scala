package overlord.Instances

import ikuy_utils.Utils
import overlord.SoftwareDefinitionTrait

trait SoftwareInstance extends InstanceTrait {
	lazy val name: String = Utils.lookupString(attributes, "name", "NONAME")
	val folder: String = ""

	override def definition: SoftwareDefinitionTrait
}
