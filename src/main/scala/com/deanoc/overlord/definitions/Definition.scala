package com.deanoc.overlord.definitions

import com.deanoc.overlord.Overlord
import com.deanoc.overlord.definitions.{HardwareDefinition, DefinitionType}
import com.deanoc.overlord.utils.{
  Variant,
  ArrayV,
  BigIntV,
  BooleanV,
  DoubleV,
  IntV,
  StringV,
  TableV
}
import com.deanoc.overlord.config._
import io.circe.{Json, JsonObject}

object Definition {
  def apply(
      config: DefinitionConfig,
      defaults: Map[String, Variant]
  ): Either[String, DefinitionTrait] = {

    val path = Overlord.catalogPath
    val `type` = DefinitionType(config.`type`)

    val mergedAttributes = if (defaults.isEmpty) {
      config.attributes
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
      defaultsAsAny ++ config.attributes
    }
    
    // Use the withAttributes method to create a new instance with updated attributes
    val configWithDefaults = config.withAttributes(mergedAttributes)

    `type` match {
      // Chip definitions
      case _: DefinitionType.RamDefinition | _: DefinitionType.CpuDefinition |
          _: DefinitionType.GraphicDefinition |
          _: DefinitionType.StorageDefinition |
          _: DefinitionType.NetDefinition | _: DefinitionType.IoDefinition |
          _: DefinitionType.OtherDefinition | _: DefinitionType.SocDefinition |
          _: DefinitionType.SwitchDefinition =>
        HardwareDefinition(`type`, configWithDefaults, path)

      // Software definitions
      case _: DefinitionType.ProgramDefinition |
          _: DefinitionType.LibraryDefinition =>
        SoftwareDefinition(`type`, configWithDefaults, path)

      // Port definitions
      case _: DefinitionType.PinGroupDefinition =>
        HardwareDefinition(`type`, configWithDefaults, path)
      case _: DefinitionType.ClockDefinition =>
        HardwareDefinition(`type`, configWithDefaults, path)

      // Board definition
      case _: DefinitionType.BoardDefinition =>
        HardwareDefinition(`type`, configWithDefaults, path)

      // Component definition
      case _: DefinitionType.ComponentDefinition =>
        Left(s"Component definitions are NOT defined this way")
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
