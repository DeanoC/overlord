package com.deanoc.overlord.definitions

import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.config.HardwareDefinitionConfig
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
    val config: HardwareDefinitionConfig,
    dependencies: Seq[String],
    boundraries: Map[String, HardwareBoundrary],
    maxInstances: Int,
    registersV: Seq[Variant]
) extends HardwareDefinition
