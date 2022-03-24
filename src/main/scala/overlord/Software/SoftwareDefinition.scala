package overlord.Software

import actions.ActionsFile
import ikuy_utils.{StringV, Utils, Variant}
import overlord.{DefinitionType, Game, SoftwareDefinitionTrait}

import java.nio.file.Path

case class SoftwareDefinition(defType: DefinitionType,
                              attributes: Map[String, Variant],
                              parameters: Map[String, Variant],
                              dependencies: Seq[String],
                              actionsFile: ActionsFile)
	extends SoftwareDefinitionTrait {
}


object SoftwareDefinition {
	def apply(table: Map[String, Variant], path: Path): Option[SoftwareDefinition] = {
		if (!table.contains("type")) {
			return None
		}

		val defTypeName = Utils.toString(table("type"))

		val software = if (!table.contains("software")) {
			val name = defTypeName.split('.')
			s"${name.last}/${name.last}.toml"
		} else {
			Utils.toString(table("software"))
		}
		val name     = if (!table.contains("name")) {
			val name = defTypeName.split('.')
			s"${name.last}"
		} else {
			Utils.toString(table("name"))
		}

		val dependencies: Seq[String] = if (table.contains("depends")) {
			val depends = Utils.toArray(table("depends"))
			depends.map(Utils.toString)
		} else {
			Seq()
		}

		val attribs = table.filter(a => a._1 match {
			case "type" | "software" | "name" | "depends" => false
			case _                                        => true
		}) ++ Map[String, Variant]("name" -> StringV(name))

		Some(SoftwareDefinition(DefinitionType(defTypeName),
		                        attribs,
		                        defTypeName,
		                        dependencies,
		                        path.resolve(software)).get)
	}

	def apply(defType: DefinitionType,
	          attributes: Map[String, Variant],
	          name: String,
	          dependencies: Seq[String],
	          spath: Path): Option[SoftwareDefinition] = {
		val path = Game.pathStack.top.resolve(spath)
		Game.pathStack.push(path.getParent)
		val result = parse(defType,
		                   attributes,
		                   name,
		                   dependencies,
		                   Utils.readToml(name, path, getClass))
		Game.pathStack.pop()
		result
	}

	private def parse(defType: DefinitionType,
	                  attributes: Map[String, Variant],
	                  name: String,
	                  dependencies: Seq[String],
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
			dependencies,
			actionsFile.get))
	}

}