package com.deanoc.overlord.definitions

import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant

import com.deanoc.overlord.config.DefinitionConfig

import java.nio.file.Path
import com.deanoc.overlord.config.HardwareDefinitionConfig
import com.deanoc.overlord.SourceLoader
import com.deanoc.overlord.config.GatewareConfig
import com.deanoc.overlord.config.PingroupsConfig

import com.deanoc.overlord.hardware.RegisterBank
import com.deanoc.overlord.instances._

import io.circe.parser.decode 
import com.deanoc.overlord.config.BoardDefinitionConfig

case class BoardDefinition(
  defType: DefinitionType,
  config: BoardDefinitionConfig,
  sourcePath: Path,
  dependencies: Seq[String],
  pinGroups: Map[String, PingroupsConfig]
) extends DefinitionTrait {

  def createInstance(
      name: String,
      instanceConfig: Map[String, Any]
  ): Either[String, InstanceTrait] = {
    Left("TODO: Implement createInstance for BoardDefinition")
    // Right(BoardInstance(
    //   name = name,
    //   definition = this,
    // ))
  }
}

/** Companion object for BoardDefinition. 
  */
object BoardDefinition {

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
      config: BoardDefinitionConfig,
      path: Path
  ): Either[String, BoardDefinition] = {
    Right(BoardDefinition(
            defType, // Pass defType directly
            config,
            path,
            Seq.empty,
            Map.empty
        ))
    }
}
