package overlord.Hardware

import gagameos._
import overlord.{ChipDefinitionTrait, DefinitionType, HardwareDefinitionTrait}

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
    * @param table
    *   The table containing chip definition data.
    * @param path
    *   The source path of the definition.
    * @return
    *   An Option containing the ChipDefinitionTrait if valid, otherwise None.
    */
  def apply(
      table: Map[String, Variant],
      path: Path
  ): Option[ChipDefinitionTrait] = {
    // Extract the type of the definition from the table
    val defTypeName = Utils.toString(table("type"))

    // Filter out reserved keys to extract attributes
    val attribs = table.filter(a =>
      a._1 match {
        case "type" | "software" | "gateware" | "hardware" | "ports" |
            "registers" | "drivers" =>
          false
        case _ => true
      }
    )

    // Ensure the definition type name has at least three components
    val name = defTypeName.split('.')
    if (
      (name(0) != "board" &&
        name(0) != "pingroup" &&
        name(0) != "other" &&
        name(0) != "clock") && name.length <= 2
    ) {
      println(s"$defTypeName must have at least 3 elements A.B.C")
      return None
    }

    // Extract register definitions
    val registers: Seq[Variant] = Utils.lookupArray(table, "registers").toSeq

    // Extract port definitions if available
    val ports = {
      if (table.contains("ports"))
        Ports(Utils.toArray(table("ports"))).map(t => t.name -> t).toMap
      else Map[String, Port]()
    }

    // Extract driver dependencies
    val dependencies: Seq[String] = if (table.contains("drivers")) {
      // Convert driver entries to a sequence of strings
      val depends = Utils.toArray(table("drivers"))
      depends.map(Utils.toString).toSeq
    } else {
      Seq()
    }

    // Create a DefinitionType object for the definition
    val defType = DefinitionType(defTypeName)

    // Check if the definition contains gateware-specific information
    if (table.contains("gateware")) {

      // Extract parameters for the gateware
      val parameters: Map[String, Variant] = if (table.contains("parameters")) {
        val params = Utils.toArray(table("parameters"))
        (for (p <- params) yield {
          val entry = Utils.toTable(p)
          Utils.lookupString(entry, "name", "NO_NAME") -> entry("value")
        }).toMap
      } else Map[String, Variant]()

      // Create a GatewareDefinition object
      val gw = table("gateware")
      GatewareDefinition(
        defType,
        attribs,
        dependencies,
        ports,
        registers,
        parameters,
        Utils.toString(gw)
      )

    } else {
      // Handle hardware definitions by extracting the maximum instances allowed
      val mi = Utils.lookupInt(table, "max_instances", 1)

      // Create a FixedHardwareDefinition object
      Some(
        FixedHardwareDefinition(
          defType,
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
