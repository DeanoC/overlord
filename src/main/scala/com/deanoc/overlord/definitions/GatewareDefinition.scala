package com.deanoc.overlord.definitions

import com.deanoc.overlord.{ChipDefinitionTrait, DefinitionType}
import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.utils.Variant

import java.nio.file.Path

/** Represents a gateware definition with associated metadata, ports, registers, and parameters.
  *
  * @param defType
  *   The type of the definition.
  * @param attributes
  *   A map of attributes associated with the definition.
  * @param dependencies
  *   A sequence of driver dependencies.
  * @param ports
  *   A map of ports defined for the hardware.
  * @param registersV
  *   A sequence of register definitions.
  * @param parameters
  *   A map of parameters for the gateware.
  * @param gatewareType
  *   The type of gateware.
  */
case class GatewareDefinition(
    defType: DefinitionType,
    attributes: Map[String, Variant],
    dependencies: Seq[String],
    ports: Map[String, Port],
    registersV: Seq[Variant],
    parameters: Map[String, Variant],
    gatewareType: String
) extends ChipDefinitionTrait {
  val maxInstances: Int = 1
  val sourcePath: Path = null // This may need to be updated based on actual requirements
}

object GatewareDefinition {
  def apply(
      defType: DefinitionType,
      attributes: Map[String, Variant],
      dependencies: Seq[String],
      ports: Map[String, Port],
      registersV: Seq[Variant],
      parameters: Map[String, Variant],
      gatewareType: String
  ): Either[String, ChipDefinitionTrait] = {
    Right(new GatewareDefinition(
      defType,
      attributes,
      dependencies,
      ports,
      registersV,
      parameters,
      gatewareType
    ))
  }
}
