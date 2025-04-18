package com.deanoc.overlord

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.hardware.{
  HardwareDefinition,
  Port,
  RegisterBank,
  Registers
}
import com.deanoc.overlord.instances._
import com.deanoc.overlord.software.SoftwareDefinition
import com.deanoc.overlord.Overlord

import java.nio.file.Path
import scala.sys.exit

import io.circe.parser.decode
import com.deanoc.overlord.config._ // Import all config case classes and decoders
import io.circe.syntax._ // Import asJson
import io.circe.{
  Json,
  JsonObject,
  Encoder
} // Import Json, JsonObject, and Encoder
import io.circe.generic.auto._ // Import auto derivation for encoders/decoders
import com.deanoc.overlord.utils.{
  ArrayV,
  BigIntV,
  BooleanV,
  DoubleV,
  IntV,
  StringV,
  TableV
}

trait DefinitionTrait {
  val defType: DefinitionType
  val attributes: Map[String, Variant]
  val sourcePath: Path
  val dependencies: Seq[String]

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Option[Map[String, Any]]
  ): Either[String, InstanceTrait]
}

object Definition {
  def apply(
      config: com.deanoc.overlord.config.DefinitionConfig,
      defaults: Map[String, Variant]
  ): Either[String, DefinitionTrait] = {

    val path = Overlord.catalogPath
    val defType = DefinitionType(config.`type`)

    // Apply defaults to config.config if needed
    // This is a simplified approach - in a more complete implementation,
    // we would merge defaults into config.config in a type-safe way
    val mergedConfig = if (defaults.isEmpty) {
      config.config
    } else {
      // Convert defaults to Map[String, Any] and merge with config.config
      val defaultsAsAny = defaults.map {
        case (k, v: StringV)  => k -> v.value
        case (k, v: IntV)     => k -> v.value
        case (k, v: BooleanV) => k -> v.value
        case (k, v: BigIntV)  => k -> v.value
        case (k, v: DoubleV)  => k -> v.value
        case (k, v: ArrayV)   => k -> v.value.toSeq
        case (k, v: TableV)   => k -> v.value
      }
      val configMap = config.config.getOrElse(Map())
      Some(defaultsAsAny ++ configMap)
    }

    defType match {
      case dt: ChipDefinitionType =>
        HardwareDefinition(defType, mergedConfig, path)
      case dt: SoftwareDefinitionType =>
        SoftwareDefinition(defType, mergedConfig, path)
      case dt: PinGroupDefinitionType =>
        HardwareDefinition(defType, mergedConfig, path)
      case dt: ClockDefinitionType =>
        HardwareDefinition(defType, mergedConfig, path)
      case dt: BoardDefinitionType =>
        HardwareDefinition(defType, mergedConfig, path)
      case null => Left(s"Unknown definition type: ${config.`type`}")
    }
  }

  // Helper function to convert Any to io.circe.Json
  def anyToJson(value: Any): Json = value match {
    case s: String  => Json.fromString(s)
    case i: Int     => Json.fromInt(i)
    case l: Long    => Json.fromLong(l)
    case d: Double  => Json.fromDoubleOrNull(d)
    case b: Boolean => Json.fromBoolean(b)
    case map: Map[_, _] =>
      Json.fromJsonObject(JsonObject.fromIterable(map.map {
        case (k: String, v) => k -> anyToJson(v)
        case (k, v) =>
          k.toString -> anyToJson(v) // Convert non-String keys to String
      }))
    case list: List[_] => Json.fromValues(list.map(anyToJson))
    case seq: Seq[_]   => Json.fromValues(seq.map(anyToJson))
    case other =>
      Json.fromString(other.toString) // Convert other types to String
  }
}

trait ChipDefinitionTrait extends DefinitionTrait {
  val ports: Map[String, Port]
  val maxInstances: Int
  // Removed ambiguous defType definition
  val attributes: Map[String, Variant]
  protected val registersV: Seq[Variant]
  var registers: Seq[RegisterBank] = Seq()

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Option[Map[String, Any]]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: RamDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into RamConfig
            io.circe.parser.decode[RamConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(ramConfig) =>
                RamInstance(name, this, ramConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode RamConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"RamDefinitionType instance $name requires configuration")
        }
      case _: CpuDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into CpuConfig
            io.circe.parser.decode[CpuConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(cpuConfig) =>
                CpuInstance(name, this, cpuConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode CpuConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"CpuDefinitionType instance $name requires configuration")
        }
      case _: IoDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into IoConfig
            io.circe.parser.decode[IoConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(ioConfig) =>
                IoInstance(name, this, ioConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode IoConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"IoDefinitionType instance $name requires configuration")
        }
      case _: PinGroupDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into PinGroupConfig
            io.circe.parser.decode[PinGroupConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(pinGroupConfig) =>
                PinGroupInstance(
                  name = name,
                  definition = this,
                  config = pinGroupConfig
                ).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode PinGroupConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(
              s"PinGroupDefinitionType instance $name requires configuration"
            )
        }
      case _: ClockDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into ClockConfig
            io.circe.parser.decode[ClockConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(clockConfig) =>
                ClockInstance(name, this, clockConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode ClockConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"ClockDefinitionType instance $name requires configuration")
        }
      case _: BoardDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into BoardConfig
            io.circe.parser.decode[BoardConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(boardConfig) =>
                BoardInstance(
                  name = name,
                  definition = this,
                  config = boardConfig
                ) // Call apply method with named parameters
              case Left(error) =>
                Left(
                  s"Failed to decode BoardConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"BoardDefinitionType instance $name requires configuration")
        }
      // For definition types without specific configs, pass the generic config map
      case _: GraphicDefinitionType => { // Manually create instance and merge attributes
        val graphic = GraphicInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        graphic.mergeAllAttributes(attribs)
        Right(graphic)
      }
      case _: StorageDefinitionType => { // Manually create instance and merge attributes
        val storage = StorageInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        storage.mergeAllAttributes(attribs)
        Right(storage)
      }
      case _: NetDefinitionType => { // Manually create instance and merge attributes
        val net = NetInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        net.mergeAllAttributes(attribs)
        Right(net)
      }
      case _: SocDefinitionType => { // Manually create instance and merge attributes
        val soc = SocInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        soc.mergeAllAttributes(attribs)
        Right(soc)
      }
      case _: SwitchDefinitionType => { // Manually create instance and merge attributes
        val sw = SwitchInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        sw.mergeAllAttributes(attribs)
        Right(sw)
      }
      case _: OtherDefinitionType => { // Manually create instance and merge attributes
        val other = OtherInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        other.mergeAllAttributes(attribs)
        Right(other)
      }
      case _ => Left(s"$defType is invalid for chip")
    }
  }
}

trait GatewareDefinitionTrait extends ChipDefinitionTrait {
  val actionsFile: ActionsFile
  val parameters: Map[String, Variant]
}

trait SoftwareDefinitionTrait extends DefinitionTrait {
  val actionsFilePath: Path
  val actionsFile: ActionsFile
  val parameters: Map[String, Variant]
  // Removed ambiguous defType definition

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Option[Map[String, Any]]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: LibraryDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into LibraryConfig
            io.circe.parser.decode[LibraryConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(libraryConfig) =>
                LibraryInstance(name, this, libraryConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode LibraryConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"LibraryDefinitionType instance $name requires configuration")
        }
      case _: ProgramDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into ProgramConfig
            io.circe.parser.decode[ProgramConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match { // Corrected decoding using helper
              case Right(programConfig) =>
                ProgramInstance(name, this, programConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]] // Cast to expected return type
              case Left(error) =>
                Left(
                  s"Failed to decode ProgramConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"ProgramDefinitionType instance $name requires configuration")
        }
      case _ => Left(s"$defType is invalid for software")
    }
  }
}

trait HardwareDefinitionTrait extends ChipDefinitionTrait
