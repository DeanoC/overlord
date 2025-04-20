package com.deanoc.overlord.definitions

import com.deanoc.overlord.Overlord
import com.deanoc.overlord.definitions.{
  HardwareDefinition,
  DefinitionType
}
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
      defaultsAsAny ++ config.config
    }

    defType match {
      // Chip definitions
      case _: DefinitionType.RamDefinition | _: DefinitionType.CpuDefinition |
           _: DefinitionType.GraphicDefinition | _: DefinitionType.StorageDefinition |
           _: DefinitionType.NetDefinition | _: DefinitionType.IoDefinition |
           _: DefinitionType.OtherDefinition | _: DefinitionType.SocDefinition |
           _: DefinitionType.SwitchDefinition =>
        HardwareDefinition(defType, mergedConfig, path)
      
      // Software definitions
      case _: DefinitionType.ProgramDefinition | _: DefinitionType.LibraryDefinition =>
        SoftwareDefinition(defType, mergedConfig, path)
      
      // Port definitions
      case _: DefinitionType.PinGroupDefinition =>
        HardwareDefinition(defType, mergedConfig, path)
      case _: DefinitionType.ClockDefinition =>
        HardwareDefinition(defType, mergedConfig, path)
      
      // Board definition
      case _: DefinitionType.BoardDefinition =>
        HardwareDefinition(defType, mergedConfig, path)
      
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
