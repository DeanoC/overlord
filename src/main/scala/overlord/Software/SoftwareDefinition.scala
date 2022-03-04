package overlord.Software

import actions.ActionsFile
import ikuy_utils.{Utils, Variant}
import overlord.{DefinitionType, Game, SoftwareDefinitionTrait}

import java.nio.file.Path

case class SoftwareDefinition(defType: DefinitionType,
                              attributes: Map[String, Variant],
                              parameters: Map[String, Variant],
                              actionsFile: ActionsFile)
	extends SoftwareDefinitionTrait {
}


object SoftwareDefinition {
	def apply(table: Map[String, Variant], path: Path): Option[SoftwareDefinition] = {
		if (!table.contains("software")) return None
		val attribs = table.filter(a => a._1 match {
			case "type" | "software" => false
			case _                   => true
		})

		val defTypeName = Utils.toString(table("type"))
		val software    = Utils.toString(table("software"))

		Some(SoftwareDefinition(DefinitionType(defTypeName),
		                        attribs,
		                        defTypeName,
		                        path.resolve(software)).get)
	}

	def apply(defType: DefinitionType,
	          attributes: Map[String, Variant],
	          name: String,
	          spath: Path): Option[SoftwareDefinition] = {
		val path = Game.pathStack.top.resolve(spath)
		Game.pathStack.push(path.getParent)
		val result = parse(defType,
		                   attributes,
		                   name,
		                   Utils.readToml(name, path, getClass))
		Game.pathStack.pop()
		result
	}

	private def parse(defType: DefinitionType,
	                  attributes: Map[String, Variant],
	                  name: String,
	                  parsed: Map[String, Variant]): Option[SoftwareDefinition] = {
		val actionsFile = ActionsFile(name, parsed)

		if (actionsFile.isEmpty) {
			println(s"Software actions file $name invalid\n")
			return None
		}

		val parameters = if (parsed.contains("parameters"))
			Utils.toTable(parsed("parameters"))
		else Map[String, Variant]()

		Some(SoftwareDefinition(
			defType,
			attributes,
			parameters,
			actionsFile.get))
	}

}