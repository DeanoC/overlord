package com.deanoc.overlord.definitions

import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant

import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.config.DefinitionConfig

import java.nio.file.Path
import com.deanoc.overlord.config.HardwareDefinitionConfig
import com.deanoc.overlord.SourceLoader
import com.deanoc.overlord.config.GatewareConfig
import com.deanoc.overlord.hardware.RegisterBank
import com.deanoc.overlord.instances._

import io.circe.parser.decode 

trait HardwareDefinition extends DefinitionTrait {
  val boundraries: Map[String, HardwareBoundrary]
  val maxInstances: Int
  protected val registersV: Seq[Variant]
  var registers: Seq[RegisterBank] = Seq()

  override def createInstance(
      name: String,
      instanceConfig: Map[String, Any]
  ): Either[String, InstanceTrait] = {
    Left("TODO: Implement createInstance for HardwareDefinition")
  }
}

/** Companion object for HardwareDefinition. Provides methods to create
  * ChipDefinitionTrait instances from input data.
  */
object HardwareDefinition {

  /** Creates a ChipDefinitionTrait from a table of data and a source path.
    *
    * @param defType
    *   The type of the definition.
    * @param config
    *   The configuration map for the definition.
    * @param path
    *   The source path of the definition.
    * @return
    *   An Either containing the ChipDefinitionTrait if valid, or an error
    *   message if invalid.
    */
  def apply(
      defType: DefinitionType, // Accept DefinitionType directly
      config: HardwareDefinitionConfig,
      path: Path
  ): Either[String, HardwareDefinition] = {

    // Extract driver dependencies from config map
    val dependencies: Seq[String] = Seq()
    /* TODO drivers
    if (configMap.contains("drivers")) {
      // Convert driver entries to a sequence of strings
      val depends = Utils.toArray(configMap("drivers"))
      depends.map(Utils.toString).toSeq
    } else {
      Seq()
    }*/
    
    // Check if the definition contains gateware-specific information in the config map
    if(config.gateware.isDefined) {
      SourceLoader.loadSource[GatewareConfig, GatewareConfig](config.gateware.get) match {
        case Right(gw) =>
          Right(GatewareDefinition(
            defType, // Pass defType directly
            path,
            config,
            dependencies,
            Map.empty,
            1,
            registersV = Seq(),
            gatewareConfig = gw
          ))
    
        case Left(e) => Left(e)
      }
  
    } else {
      // Create a FixedHardwareDefinition object
      Right(
        FixedHardwareDefinition(
          defType, // Pass defType directly
          path,
          config,
          dependencies,
          Map.empty,
          config.max_instances, // max_instances is now an Int, no need for getOrElse
          Seq()
        )
      )
    }
  }
}
