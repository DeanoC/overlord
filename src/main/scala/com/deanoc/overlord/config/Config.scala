package com.deanoc.overlord.config

import io.circe.{Decoder, Json, HCursor}
import io.circe.generic.semiauto._
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import scala.collection.mutable
import com.deanoc.overlord.config.CirceDefaults.withDefault

object CirceDefaults {
  def withDefault[T](cursor: HCursor, field: String, default: T)(implicit decoder: Decoder[T]): Decoder.Result[T] = {
    cursor.downField(field).as[T].orElse(Right(default))
  }
  
  def withDefaultOption[T](cursor: HCursor, field: String)(implicit decoder: Decoder[T]): Decoder.Result[Option[T]] = {
    cursor.downField(field).as[Option[T]].orElse(Right(None))
  }
}

// Represents a memory range within a RAM definition
case class MemoryRangeConfig(
  address: String, // Assuming address is represented as a hex string in YAML
  size: String     // Assuming size is represented as a hex string in YAML
) derives Decoder
// Represents the configuration for an IO definition
case class IoConfig(
  visible_to_software: Boolean
) derives Decoder

// Represents the configuration for a PinGroup definition
case class PinGroupConfig(
  pins: List[String],
  direction: String // Assuming direction is a string like "input" or "output"
) derives Decoder

// Represents the configuration for a Clock definition
case class ClockConfig(
  frequency: String // Assuming frequency is a string like "100MHz"
) derives Decoder

// Represents the configuration for a Program definition
case class ProgramConfig(
  dependencies: List[String]
) derives Decoder

// Represents the configuration for a Library definition
case class LibraryConfig(
  dependencies: List[String]
) derives Decoder

// Represents a clock within a Board definition
case class BoardClockConfig(
  name: String,
  frequency: String // Assuming frequency is a string like "100MHz"
) derives Decoder

// Represents the configuration for a Board definition
case class BoardConfig(
  board_type: String,
  clocks: List[BoardClockConfig]
) derives Decoder

// Custom decoder for Map[String, Any]
object CustomDecoders {
  implicit val decodeMapStringAny: Decoder[Map[String, Any]] = new Decoder[Map[String, Any]] {
    def apply(c: HCursor): Decoder.Result[Map[String, Any]] = {
      c.as[Map[String, Json]].map { jsonMap =>
        jsonMap.map { case (k, v) =>
          k -> (v.asString.orElse(v.asBoolean.map(_.toString))
                .orElse(v.asNumber.map(_.toString))
                .orElse(v.asArray.map(_.toString))
                .orElse(v.asObject.map(_.toString))
                .getOrElse(v.toString))
        }
      }
    }
  }
}
// Define an enum for source types
sealed trait SourceType
object SourceType {
  case object Git extends SourceType
  case object Local extends SourceType
  case object Fetch extends SourceType
  case object Inline extends SourceType

  def fromString(str: String): Option[SourceType] = str.toLowerCase match {
    case "git" => Some(Git)
    case "local" => Some(Local)
    case "fetch" => Some(Fetch)
    case "inline" => Some(Inline)
    case _ => None
  }
  
  def toString(sourceType: SourceType): String = sourceType match {
    case Git => "git"
    case Local => "local"
    case Fetch => "fetch"
    case Inline => "inline"
  }
  
  implicit val decoder: Decoder[SourceType] = Decoder.decodeString.emap { str =>
    fromString(str).toRight(s"Invalid source type: $str")
  }
}

case class SourceConfig(
  `type`: SourceType,
  url: Option[String] = None,
  path: Option[String] = None,
  inline: Option[Json] = None,
)

object SourceConfig {
  implicit val decoder: Decoder[SourceConfig] = new Decoder[SourceConfig] {
    def apply(c: HCursor): Decoder.Result[SourceConfig] = {
      for {
        t <- c.downField("type").as[SourceType]
        u = t match {
          case SourceType.Git | SourceType.Fetch => c.downField("url").as[String].toOption
          case _                                 => None
        }
        p = t match {
          case SourceType.Local => c.downField("path").as[String].toOption
          case _                => None
        }
        i = t match {
          case SourceType.Inline => c.downField("inline").as[Json].toOption
          case _                 => None
        }
      } yield SourceConfig(t, u, p, i)
    }
  }
}
case class FieldConfig(
  bits: String, // Bit range "high:low"
  name: String, // Field identifier
  `type`: String, // Access type: raz/rw/ro/wo/mixed
  shortdesc: Option[String] = None, // Brief functional description
  longdesc: Option[String] = None // Detailed technical documentation
) derives Decoder

case class RegisterConfig(
  default: String, // Power-on value (e.g., "0x00000000")
  description: String, // Functional purpose
  field: List[FieldConfig], // Bitfield definitions
  name: String, // Register name (e.g., "MCU_RESET")
  offset: String, // Address offset from bank base
  `type`: String, // Access type: mixed/rw/ro/wo
  width: String // Bit width (e.g., "32")
) derives Decoder

case class PortConfig(
  name: String,
  width: String,
  direction: String
) derives Decoder

case class InfoConfig(
  name: String = "",
  version: Option[String] = None,
  author: Option[String] = None,
  description: Option[String] = None
) derives Decoder

// configuration specific to gateware
case class GatewareConfig(
  test: String = "",
  parameters: Map[String, Any] = Map.empty,
)
 
object GatewareConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[GatewareConfig] = new Decoder[GatewareConfig] {
    def apply(c: HCursor): Decoder.Result[GatewareConfig] = {
      for {
        test <- c.downField("test").as[String]
        parameters <- withDefault(c, "parameters", Map.empty[String, Any])
      } yield GatewareConfig(test, parameters)
    }
  }
} 

// Common trait for catalog/component file configuration
trait FileConfigBase {
  def defaults: Map[String, Any]
  def catalogs: List[SourceConfig]
  def definitions: List[DefinitionConfig]
}

// Represents the top-level structure of a catalog YAML file
case class CatalogFileConfig(
  defaults: Map[String, Any] = Map.empty,
  catalogs: List[SourceConfig] = List.empty,
  definitions: List[DefinitionConfig] = List.empty
) extends FileConfigBase

object CatalogFileConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[CatalogFileConfig] = new Decoder[CatalogFileConfig] {
    def apply(c: HCursor): Decoder.Result[CatalogFileConfig] = {
      for {
        defaults <- c.downField("defaults").as[Option[Map[String, Any]]].map(_.getOrElse(Map.empty))
        catalogs <- c.downField("catalogs").as[Option[List[SourceConfig]]].map(_.getOrElse(List.empty))
        definitions <- c.downField("definitions").as[Option[List[DefinitionConfig]]].map(_.getOrElse(List.empty))
      } yield CatalogFileConfig(defaults, catalogs, definitions)
    }
  }
}

// Represents the top-level structure of the project YAML file
case class ComponentFileConfig(
  info: InfoConfig,
  components: List[SourceConfig] = List.empty,
  instances: List[InstanceConfig] = List.empty,
  connections: List[ConnectionConfig] = List.empty,
  defaults: Map[String, Any] = Map.empty,
  catalogs: List[SourceConfig] = List.empty,
  definitions: List[DefinitionConfig] = List.empty
) extends FileConfigBase

object ComponentFileConfig {
  import CustomDecoders._
  
  // Custom decoder that handles the structure
  implicit val decoder: Decoder[ComponentFileConfig] = new Decoder[ComponentFileConfig] {
    def apply(c: HCursor): Decoder.Result[ComponentFileConfig] = {
      for {
        info <- c.downField("info").as[InfoConfig]
        components <- c.downField("components").as[Option[List[SourceConfig]]].map(_.getOrElse(List.empty))
        instances <- c.downField("instances").as[Option[List[InstanceConfig]]].map(_.getOrElse(List.empty))
        connections <- c.downField("connections").as[Option[List[ConnectionConfig]]].map(_.getOrElse(List.empty))
        defaults <- c.downField("defaults").as[Option[Map[String, Any]]].map(_.getOrElse(Map.empty))
        catalogs <- c.downField("catalogs").as[Option[List[SourceConfig]]].map(_.getOrElse(List.empty))
        definitions <- c.downField("definitions").as[Option[List[DefinitionConfig]]].map(_.getOrElse(List.empty))
      } yield ComponentFileConfig(
        info, components, instances, connections, defaults, catalogs, definitions
      )
    }
  }
}
