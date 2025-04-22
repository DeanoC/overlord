package com.deanoc.overlord.definitions

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.utils.Utils.VariantTable
import com.deanoc.overlord.{Overlord}
import com.deanoc.overlord.config.DefinitionConfig

import java.nio.file.{Path, Paths}
import com.deanoc.overlord.config.ConfigPaths
import com.deanoc.overlord.config.GatewareConfig

/** Represents a gateware definition with associated metadata, ports, registers, and parameters.
  *
  * @param defType
  *   The type of the definition.``
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

    gatewareConfig: GatewareConfig
//    parameters: Map[String, Variant],
//    actionsFile: ActionsFile
) extends HardwareDefinition {}
