package overlord.Chip

import actions.ActionsFile
import ikuy_utils.Utils.VariantTable
import ikuy_utils.{Utils, Variant}
import overlord.{DefinitionType, Game, GatewareDefinitionTrait}

import java.nio.file.Path

case class GatewareDefinition(defType: DefinitionType,
                              sourcePath: Path,
                              attributes: Map[String, Variant],
                              ports: Map[String, Port],
                              maxInstances: Int = 1,
                              registersV: Seq[Variant],
                              parameters: Map[String, Variant],
                              actionsFile: ActionsFile
                             ) extends GatewareDefinitionTrait {
}

object GatewareDefinition {
	def apply(table: VariantTable): Option[GatewareDefinition] = {
		if (!table.contains("gateware")) return None

		val defTypeName = Utils.toString(table("type"))

		val attribs = table.filter(a => a._1 match {
			case "type" | "gateware" | "ports" | "registers" => false
			case _                                           => true
		})

		val name = defTypeName.split('.')

		val registers: Seq[Variant] = Utils.lookupArray(table, "registers").toSeq

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
		                        Utils.toString(table("gateware"))).get)

	}

	def apply(defType: DefinitionType,
	          attributes: Map[String, Variant],
	          ports: Map[String, Port],
	          registers: Seq[Variant],
	          fileName: String): Option[GatewareDefinition] = {
		val fileNameAlone = Path.of(fileName).getFileName
		Game.pushCatalogPath(Path.of(fileName).getParent)
		val result = parse(defType,
		                   attributes,
		                   ports,
		                   registers,
		                   fileName,
		                   Utils.readToml(Game.catalogPath.resolve(fileNameAlone)))
		Game.popCatalogPath()
		result
	}

	private def parse(defType: DefinitionType,
	                  attributes: Map[String, Variant],
	                  iports: Map[String, Port],
	                  registers: Seq[Variant],
	                  fileName: String,
	                  parsed: Map[String, Variant]): Option[GatewareDefinition] = {

		val actionsFile = ActionsFile(fileName, parsed)

		if (actionsFile.isEmpty) {
			println(s"Gateware actions file $fileName invalid\n")
			return None
		}

		val ports = if (parsed.contains("ports"))
			iports ++ Ports(Utils.toArray(parsed("ports"))).map(t => t.name -> t).toMap
		else iports

		val parameters = if (parsed.contains("parameters"))
			Utils.toTable(parsed("parameters"))
		else Map[String, Variant]()


		Some(GatewareDefinition(defType,
		                        Game.catalogPath,
		                        attributes,
		                        ports,
		                        1,
		                        registers,
		                        parameters,
		                        actionsFile.get))
	}

}