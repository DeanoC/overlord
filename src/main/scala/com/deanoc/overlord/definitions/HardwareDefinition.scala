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
import com.deanoc.overlord.config.IoConfig
import com.deanoc.overlord.config.PinGroupConfig
import com.deanoc.overlord.config.ClockConfig
import com.deanoc.overlord.config.BoardConfig
import com.deanoc.overlord.instances._

import io.circe.parser.decode 

trait HardwareDefinition extends DefinitionTrait {
  val boundraries: Map[String, HardwareBoundrary]
  val maxInstances: Int
  protected val registersV: Seq[Variant]
  var registers: Seq[RegisterBank] = Seq()

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Map[String, Any]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: DefinitionType.RamDefinition =>
        val ram = new RamInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        ram.mergeAllAttributes(attribs)
        Right(ram)
      case _: DefinitionType.CpuDefinition =>
        val cpu = new CpuInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        cpu.mergeAllAttributes(attribs)
        Right(cpu)

      case _: DefinitionType.IoDefinition =>
        decode[IoConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(ioConfig) =>
            IoInstance(name, this, ioConfig).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode IoConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.PinGroupDefinition =>
        decode[PinGroupConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(pinGroupConfig) =>
            PinGroupInstance(
              name = name,
              definition = this,
              config = pinGroupConfig
            ).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode PinGroupConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.ClockDefinition =>
        decode[ClockConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(clockConfig) =>
            ClockInstance(name, this, clockConfig).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode ClockConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.BoardDefinition =>
        decode[BoardConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(boardConfig) =>
            BoardInstance(
              name = name,
              definition = this.asInstanceOf[BoardDefinition],
              config = boardConfig
            )
          case Left(error) =>
            Left(
              s"Failed to decode BoardConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.GraphicDefinition => {
        val graphic = GraphicInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        graphic.mergeAllAttributes(attribs)
        Right(graphic)
      }
      case _: DefinitionType.StorageDefinition => {
        val storage = StorageInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        storage.mergeAllAttributes(attribs)
        Right(storage)
      }
      case _: DefinitionType.NetDefinition => {
        val net = NetInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        net.mergeAllAttributes(attribs)
        Right(net)
      }
      case _: DefinitionType.SocDefinition => {
        val soc = SocInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        soc.mergeAllAttributes(attribs)
        Right(soc)
      }
      case _: DefinitionType.SwitchDefinition => {
        val sw = SwitchInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        sw.mergeAllAttributes(attribs)
        Right(sw)
      }
      case _: DefinitionType.OtherDefinition => {
        val other = OtherInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        other.mergeAllAttributes(attribs)
        Right(other)
      }
      case _ => Left(s"$defType is invalid for chip")
    }
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
