package com.deanoc.overlord.hardware

import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.{
  ChipDefinitionTrait,
  DefinitionType,
  HardwareDefinitionTrait
}

import java.nio.file.Path

/** Represents a hardware definition with associated metadata, ports, and
  * registers.
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
  *   The maximum number of instances allowed.
  * @param registersV
  *   A sequence of register definitions.
  */
case class FixedHardwareDefinition(
    defType: DefinitionType,
    sourcePath: Path,
    attributes: Map[String, Variant],
    dependencies: Seq[String],
    ports: Map[String, Port],
    maxInstances: Int,
    registersV: Seq[Variant]
) extends HardwareDefinitionTrait

/** Companion object for HardwareDefinition. Provides methods to create
  * ChipDefinitionTrait instances from input data.
  */
object HardwareDefinition {

  /** Creates a ChipDefinitionTrait from a table of data and a source path.
    *
    * @param defType
    *   The type of the definition.
    * @param config
    *   The optional configuration map for the definition.
    * @param path
    *   The source path of the definition.
    * @return
    *   An Either containing the ChipDefinitionTrait if valid, or an error
    *   message if invalid.
    */
  def apply(
      defType: DefinitionType, // Accept DefinitionType directly
      config: Option[Map[String, Any]], // Accept Option[Map[String, Any]] for config
      path: Path
  ): Either[String, ChipDefinitionTrait] = {
    // Convert config Option[Map[String, Any]] to Map[String, Variant]
    val configMap: Map[String, Variant] = config.getOrElse(Map()).map { case (k, v) =>
      k -> Utils.toVariant(v) // Convert Any to Variant
    }

    // Filter out reserved keys from the config map to extract attributes
    val attribs = configMap.filter(a =>
      a._1 match {
        case "type" | "software" | "gateware" | "hardware" | "ports" |
            "registers" | "drivers" =>
          false
        case _ => true
      }
    )

    // Extract register definitions from config map
    val registers: Seq[Variant] = Utils.lookupArray(configMap, "registers").toSeq

    // Extract port definitions from config map if available
    val ports = {
      if (configMap.contains("ports"))
        Ports(Utils.toArray(configMap("ports"))).map(t => t.name -> t).toMap
      else Map[String, Port]()
    }

    // Extract driver dependencies from config map
    val dependencies: Seq[String] = if (configMap.contains("drivers")) {
      // Convert driver entries to a sequence of strings
      val depends = Utils.toArray(configMap("drivers"))
      depends.map(Utils.toString).toSeq
    } else {
      Seq()
    }

    // Check if the definition contains gateware-specific information in the config map
    if (configMap.contains("gateware")) {

      // Extract parameters for the gateware from config map
      val parameters: Map[String, Variant] =
        if (configMap.contains("parameters")) {
          val params = Utils.toArray(configMap("parameters"))
          (for (p <- params) yield {
            val entry = Utils.toTable(p)
            Utils.lookupString(entry, "name", "NO_NAME") -> entry("value")
          }).toMap
        } else Map[String, Variant]()

      // Create a GatewareDefinition object
      val gw = configMap("gateware")
      // Call the second GatewareDefinition.apply method with the correct parameter order
      GatewareDefinition(
        defType, // Pass defType directly
        attribs,
        dependencies,
        ports,
        registers,
        parameters,
        Utils.toString(gw)
      )

    } else {
      // Handle hardware definitions by extracting the maximum instances allowed from config map
      val mi = Utils.lookupInt(configMap, "max_instances", 1)

      // Create a FixedHardwareDefinition object
      Right(
        FixedHardwareDefinition(
          defType, // Pass defType directly
          path,
          attribs,
          dependencies,
          ports,
          mi,
          registers
        )
      )
    }
  }
}
