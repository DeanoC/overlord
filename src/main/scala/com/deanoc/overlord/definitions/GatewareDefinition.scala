package com.deanoc.overlord.definitions

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.hardware.{Port, Ports}
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.utils.Utils.VariantTable
import com.deanoc.overlord.{Overlord}
import com.deanoc.overlord.config.DefinitionConfig

import java.nio.file.{Path, Paths}

/** Represents a gateware definition with associated metadata, ports, registers, and parameters.
  *
  * @param defType
  *   The type of the definition.
  * @param sourcePath
  *   The source path of the definition.
  * @param attributes
  *   A map of attributes associated with the definition.
  * @param dependencies
  *   A sequence of driver dependencies.
  * @param ports
  *   A map of ports defined for the hardware.
  * @param maxInstances
  *   The maximum number of instances allowed (default is 1).
  * @param registersV
  *   A sequence of register definitions.
  * @param parameters
  *   A map of parameters for the gateware.
  * @param actionsFile
  *   The actions file associated with the gateware.
  */
case class GatewareDefinition(
    defType: DefinitionType,
    sourcePath: Path,
    config: DefinitionConfig,
    dependencies: Seq[String],
    ports: Map[String, Port],
    maxInstances: Int = 1,
    registersV: Seq[Variant],
    parameters: Map[String, Variant],
    actionsFile: ActionsFile
) extends GatewareDefinitionTrait {}

/** Companion object for GatewareDefinition. Provides methods to create
  * GatewareDefinition instances from various inputs.
  */
object GatewareDefinition {

  /** Creates a GatewareDefinition from a VariantTable.
    *
    * @param table
    *   The table containing gateware definition data.
    * @return
    *   An Either containing the GatewareDefinition if valid, or an error
    *   message if invalid.
    */
  /*def apply(table: VariantTable): Either[String, GatewareDefinition] = {
    if (!table.contains("gateware"))
      return Left("Table does not contain a 'gateware' field")

    val defTypeName = Utils.toString(table("type"))

    val attribs = table.filter(a =>
      a._1 match {
        case "type" | "gateware" | "ports" | "registers" | "parameters" |
            "drivers" =>
          false
        case _ => true
      }
    )

    val dependencies: Seq[String] = if (table.contains("drivers")) {
      val depends = Utils.toArray(table("drivers"))
      depends.map(Utils.toString).toSeq
    } else {
      Seq()
    }
    val registers: Seq[Variant] = Utils.lookupArray(table, "registers").toSeq

    val ports =
      if (table.contains("ports"))
        Ports(Utils.toArray(table("ports"))).map(t => t.name -> t).toMap
      else Map[String, Port]()
    val parameters: Map[String, Variant] =
      if (table.contains("parameters")) Utils.toTable(table("parameters"))
      else Map[String, Variant]()

    val defType = DefinitionType(defTypeName)

    parse(
      defType,
      attribs,
      dependencies,
      ports,
      registers,
      parameters,
      Utils.toString(table("gateware")),
      table
    )
  }*/

  /** Creates a GatewareDefinition by parsing a file and combining it with
    * provided metadata.
    *
    * @param defType
    *   The type of the definition.
    * @param attributes
    *   A map of attributes associated with the definition.
    * @param dependencies
    *   A sequence of driver dependencies.
    * @param ports
    *   A map of ports defined for the gateware.
    * @param registers
    *   A sequence of register definitions.
    * @param parameters
    *   A map of parameters for the gateware.
    * @param fileName
    *   The name of the file containing additional gateware data.
    * @return
    *   An Either containing the GatewareDefinition if valid, or an error
    *   message if invalid.
    */
  def apply(
      defType: DefinitionType,
      config: DefinitionConfig,
      dependencies: Seq[String],
      ports: Map[String, Port],
      registers: Seq[Variant],
      parameters: Map[String, Variant],
      fileName: String
  ): Either[String, GatewareDefinition] = {
    val fileNameAlone = Paths.get(fileName).getFileName
    Overlord.pushCatalogPath(Paths.get(fileName))
    val yaml = Utils.readYaml(Overlord.catalogPath.resolve(fileNameAlone))
    val result = parse(
      defType,
      config,
      dependencies,
      ports,
      registers,
      parameters,
      fileName,
      yaml
    )
    Overlord.popCatalogPath()
    result
  }

  /** Parses a gateware definition from a file and combines it with provided
    * metadata.
    *
    * @param defType
    *   The type of the definition.
    * @param attributes
    *   A map of attributes associated with the definition.
    * @param dependencies
    *   A sequence of driver dependencies.
    * @param iports
    *   A map of initial ports.
    * @param registers
    *   A sequence of register definitions.
    * @param parameters
    *   A map of parameters for the gateware.
    * @param fileName
    *   The name of the file containing additional gateware data.
    * @param parsed
    *   The parsed data from the file.
    * @return
    *   An Either containing the GatewareDefinition if valid, or an error
    *   message if invalid.
    */
  private def parse(
      defType: DefinitionType,
      config: DefinitionConfig,
      dependencies: Seq[String],
      iports: Map[String, Port],
      registers: Seq[Variant],
      parameters: Map[String, Variant],
      fileName: String,
      parsed: Map[String, Variant]
  ): Either[String, GatewareDefinition] = {

    // ActionsFile.createActionsFile needs to be updated to use Either, but for now we'll adapt it
    val actionsFile = ActionsFile.createActionsFile(fileName, parsed)

    if (actionsFile.isEmpty) {
      Left(s"Gateware actions file $fileName invalid")
    } else {
      val ports =
        if (parsed.contains("ports"))
          iports ++ Ports(Utils.toArray(parsed("ports")))
            .map(t => t.name -> t)
            .toMap
        else iports

      val combinedParameters =
        if (parsed.contains("parameters"))
          Utils.toTable(parsed("parameters")) ++ parameters
        else parameters

      Right(
        GatewareDefinition(
          defType,
          Overlord.catalogPath,
          config,
          dependencies,
          ports,
          1,
          registers,
          combinedParameters,
          actionsFile.get
        )
      )
    }
  }
}
