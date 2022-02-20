package overlord.Chip

import actions.ActionsFile
import ikuy_utils.Utils.VariantTable
import ikuy_utils.{Utils, Variant}
import overlord.{DefinitionType, Game, GatewareDefinitionTrait}

import java.nio.file.Path

case class GatewareDefinition(defType: DefinitionType,
                              attributes: Map[String, Variant],
                              ports: Map[String, Port],
                              registers: Option[Registers],
                              parameters: Map[String, Variant],
                              actionsFile: ActionsFile
                             ) extends GatewareDefinitionTrait {
}

object GatewareDefinition {
	def apply(table: VariantTable, path: Path): Option[GatewareDefinition] = {
		if (!table.contains("gateware")) return None

		val defTypeName = Utils.toString(table("type"))

		val attribs = table.filter(a => a._1 match {
			case "type" | "gateware" | "ports" | "registers" => false
			case _                                           => true
		})

		val name = defTypeName.split('.')

		val registers = if (table.contains("registers"))
			Some(Registers(Utils.toArray(table("registers")).toSeq, path))
		else None

		val ports = {
			if (table.contains("ports"))
				Ports(Utils.toArray(table("ports"))).map(t => t.name -> t).toMap
			else Map[String, Port]()
		}

		val defType = DefinitionType(defTypeName)

		Some(GatewareDefinition(defType,
		                        attribs,
		                        ports,
		                        registers,
		                        name.last,
		                        path.resolve(Utils.toString(table("gateware")))).get)

	}

	def apply(defType: DefinitionType,
	          attributes: Map[String, Variant],
	          ports: Map[String, Port],
	          registers: Option[Registers],
	          name: String,
	          spath: Path): Option[GatewareDefinition] = {
		val path = Game.pathStack.top.resolve(spath)
		Game.pathStack.push(path.getParent)
		val result = parse(defType,
		                   attributes,
		                   ports,
		                   registers,
		                   name,
		                   Utils.readToml(name, path, getClass))
		Game.pathStack.pop()
		result
	}

	private def parse(defType: DefinitionType,
	                  attributes: Map[String, Variant],
	                  iports: Map[String, Port],
	                  registers: Option[Registers],
	                  name: String,
	                  parsed: Map[String, Variant]): Option[GatewareDefinition] = {

		val actionsFile = ActionsFile(name, parsed)

		if (actionsFile.isEmpty) {
			println(s"Gateware actions file $name invalid\n")
			return None
		}

		val ports = if (parsed.contains("ports"))
			iports ++ Ports(Utils.toArray(parsed("ports"))).map(t => t.name -> t).toMap
		else iports

		val parameters = if (parsed.contains("parameters"))
			Utils.toTable(parsed("parameters"))
		else Map[String, Variant]()


		Some(GatewareDefinition(
			defType,
			attributes,
			ports,
			registers,
			parameters,
			actionsFile.get))
	}

}