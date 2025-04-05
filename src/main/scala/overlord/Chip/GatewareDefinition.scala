package overlord.Chip

import actions.ActionsFile
import gagameos.Utils.VariantTable
import gagameos.{Utils, Variant}
import overlord.{DefinitionType, Game, GatewareDefinitionTrait}

import java.nio.file.{Path, Paths}

case class GatewareDefinition(defType: DefinitionType,
                              sourcePath: Path,
                              attributes: Map[String, Variant],
                              dependencies: Seq[String],
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
			case "type" | "gateware" | "ports" | "registers" | "parameters" | "drivers" => false
			case _                                                                      => true
		})

		val dependencies: Seq[String]  = if (table.contains("drivers")) {
			val depends = Utils.toArray(table("drivers"))
			depends.map(Utils.toString).toSeq
		} else {
			Seq()
		}
		val registers   : Seq[Variant] = Utils.lookupArray(table, "registers").toSeq

		val ports                            =
			if (table.contains("ports")) Ports(Utils.toArray(table("ports"))).map(t => t.name -> t).toMap
			else Map[String, Port]()
		val parameters: Map[String, Variant] =
			if (table.contains("parameters")) Utils.toTable(table("parameters"))
			else Map[String, Variant]()

		val defType = DefinitionType(defTypeName)

		Some(GatewareDefinition(defType,
		                        attribs,
		                        dependencies,
		                        ports,
		                        registers,
		                        parameters,
		                        Utils.toString(table("gateware"))).get)

	}

	def apply(defType: DefinitionType,
	          attributes: Map[String, Variant],
	          dependencies: Seq[String],
	          ports: Map[String, Port],
	          registers: Seq[Variant],
	          parameters: Map[String, Variant],
	          fileName: String): Option[GatewareDefinition] = {
		val fileNameAlone = Paths.get(fileName).getFileName
		Game.pushCatalogPath(Paths.get(fileName))
		val result = parse(defType,
		                   attributes,
		                   dependencies,
		                   ports,
		                   registers,
		                   parameters,
		                   fileName,
		                   Utils.readToml(Game.catalogPath.resolve(fileNameAlone)))
		Game.popCatalogPath()
		result
	}

	private def parse(defType: DefinitionType,
	                  attributes: Map[String, Variant],
	                  dependencies: Seq[String],
	                  iports: Map[String, Port],
	                  registers: Seq[Variant],
	                  parameters: Map[String, Variant],
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

		val combinedParameters = if (parsed.contains("parameters"))
			Utils.toTable(parsed("parameters")) ++ parameters
		else parameters

		Some(GatewareDefinition(defType,
		                        Game.catalogPath,
		                        attributes,
		                        dependencies,
		                        ports,
		                        1,
		                        registers,
		                        combinedParameters,
		                        actionsFile.get))
	}

}