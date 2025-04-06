package overlord.Hardware

import actions.ActionsFile
import gagameos.Utils.VariantTable
import gagameos.{Utils, Variant}
import overlord.{DefinitionType, Project, GatewareDefinitionTrait}

import java.nio.file.{Path, Paths}

/**
 * Represents a gateware definition with associated metadata, parameters, and actions.
 *
 * @param defType       The type of the definition.
 * @param sourcePath    The source path of the definition.
 * @param attributes    A map of attributes associated with the definition.
 * @param dependencies  A sequence of driver dependencies.
 * @param ports         A map of ports defined for the gateware.
 * @param maxInstances  The maximum number of instances allowed (default is 1).
 * @param registersV    A sequence of register definitions.
 * @param parameters    A map of parameters for the gateware.
 * @param actionsFile   The actions file associated with the gateware.
 */
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

/**
 * Companion object for GatewareDefinition.
 * Provides methods to create GatewareDefinition instances from various inputs.
 */
object GatewareDefinition {

	/**
	 * Creates a GatewareDefinition from a VariantTable.
	 *
	 * @param table The table containing gateware definition data.
	 * @return An Option containing the GatewareDefinition if valid, otherwise None.
	 */
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

	/**
	 * Creates a GatewareDefinition by parsing a file and combining it with provided metadata.
	 *
	 * @param defType       The type of the definition.
	 * @param attributes    A map of attributes associated with the definition.
	 * @param dependencies  A sequence of driver dependencies.
	 * @param ports         A map of ports defined for the gateware.
	 * @param registers     A sequence of register definitions.
	 * @param parameters    A map of parameters for the gateware.
	 * @param fileName      The name of the file containing additional gateware data.
	 * @return An Option containing the GatewareDefinition if valid, otherwise None.
	 */
	def apply(defType: DefinitionType,
	          attributes: Map[String, Variant],
	          dependencies: Seq[String],
	          ports: Map[String, Port],
	          registers: Seq[Variant],
	          parameters: Map[String, Variant],
	          fileName: String): Option[GatewareDefinition] = {
		val fileNameAlone = Paths.get(fileName).getFileName
		Project.pushCatalogPath(Paths.get(fileName))
		val result = parse(defType,
		                   attributes,
		                   dependencies,
		                   ports,
		                   registers,
		                   parameters,
		                   fileName,
		                   Utils.readYaml(Project.catalogPath.resolve(fileNameAlone)))
		Project.popCatalogPath()
		result
	}

	/**
	 * Parses a gateware definition from a file and combines it with provided metadata.
	 *
	 * @param defType       The type of the definition.
	 * @param attributes    A map of attributes associated with the definition.
	 * @param dependencies  A sequence of driver dependencies.
	 * @param iports        A map of initial ports.
	 * @param registers     A sequence of register definitions.
	 * @param parameters    A map of parameters for the gateware.
	 * @param fileName      The name of the file containing additional gateware data.
	 * @param parsed        The parsed data from the file.
	 * @return An Option containing the GatewareDefinition if valid, otherwise None.
	 */
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
		                        Project.catalogPath,
		                        attributes,
		                        dependencies,
		                        ports,
		                        1,
		                        registers,
		                        combinedParameters,
		                        actionsFile.get))
	}

}