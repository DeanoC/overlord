package overlord.Software

import ikuy_utils.{Utils, Variant}
import overlord.{DefinitionTrait, DefinitionType}
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class SoftwareDefinition(defType: DefinitionType,
                              attributes: Map[String, Variant])
	extends DefinitionTrait {

	override def createInstance(name: String,
	                            attribs: Map[String, Variant]): Option[InstanceTrait] = ???
}


object SoftwareDefinition {
	def apply(table: Map[String, Variant], path: Path): SoftwareDefinition = {
		val defTypeName = Utils.toString(table("type"))
		SoftwareDefinition(DefinitionType(defTypeName), table)
	}
}