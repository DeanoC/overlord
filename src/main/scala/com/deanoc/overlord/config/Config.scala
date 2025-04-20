package com.deanoc.overlord.config

import io.circe.{Decoder, Json, HCursor}
import io.circe.generic.semiauto._
  
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
)
object MemoryRangeConfig {
  implicit val decoder: Decoder[MemoryRangeConfig] = deriveDecoder[MemoryRangeConfig]
}

// Represents the configuration for a RAM definition
case class RamConfig(
  ranges: List[MemoryRangeConfig]
)
object RamConfig {
  implicit val decoder: Decoder[RamConfig] = deriveDecoder[RamConfig]
}

// Represents the configuration for a CPU definition
case class CpuConfig(
  core_count: Int,
  triple: String
)
object CpuConfig {
  implicit val decoder: Decoder[CpuConfig] = deriveDecoder[CpuConfig]
}

// Represents the configuration for an IO definition
case class IoConfig(
  visible_to_software: Boolean
)
object IoConfig {
  implicit val decoder: Decoder[IoConfig] = deriveDecoder[IoConfig]
}

// Represents the configuration for a PinGroup definition
case class PinGroupConfig(
  pins: List[String],
  direction: String // Assuming direction is a string like "input" or "output"
)
object PinGroupConfig {
  implicit val decoder: Decoder[PinGroupConfig] = deriveDecoder[PinGroupConfig]
}

// Represents the configuration for a Clock definition
case class ClockConfig(
  frequency: String // Assuming frequency is a string like "100MHz"
)
object ClockConfig {
  implicit val decoder: Decoder[ClockConfig] = deriveDecoder[ClockConfig]
}

// Represents the configuration for a Program definition
case class ProgramConfig(
  dependencies: List[String]
)
object ProgramConfig {
  implicit val decoder: Decoder[ProgramConfig] = deriveDecoder[ProgramConfig]
}

// Represents the configuration for a Library definition
case class LibraryConfig(
  dependencies: List[String]
)
object LibraryConfig {
  implicit val decoder: Decoder[LibraryConfig] = deriveDecoder[LibraryConfig]
}

// Represents a clock within a Board definition
case class BoardClockConfig(
  name: String,
  frequency: String // Assuming frequency is a string like "100MHz"
)
object BoardClockConfig {
  implicit val decoder: Decoder[BoardClockConfig] = deriveDecoder[BoardClockConfig]
}

// Represents the configuration for a Board definition
case class BoardConfig(
  board_type: String,
  clocks: List[BoardClockConfig]
)
object BoardConfig {
  implicit val decoder: Decoder[BoardConfig] = deriveDecoder[BoardConfig]
}

// Note: Case classes for Graphic, Storage, Net, Soc, Switch, and Other
// Definition Types are not included as their attributes are marked as TBD
// in the documentation.

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

// Represents a single instance in the project file
case class InstanceConfig(
  name: String,
  `type`: String, // Use backticks for type as it's a Scala keyword
  config: Option[Map[String, Any]] = None // Flexible for various config types
)
object InstanceConfig {
  import CustomDecoders._
  implicit val decoder: Decoder[InstanceConfig] = deriveDecoder[InstanceConfig]
}

// Base trait for all connection configurations
sealed trait ConnectionConfig {
  def `type`: String
  def connection: String
}
object ConnectionConfig {
  implicit val decoder: Decoder[ConnectionConfig] = (c: HCursor) => {
    for {
      connType <- c.downField("type").as[String]
      result <- connType match {
        case "bus" => c.as[BusConnectionConfig]
        case "port" => c.as[PortConnectionConfig]
        case "port_group" => c.as[PortGroupConnectionConfig]
        case "clock" => c.as[ClockConnectionConfig]
        case "logical" => c.as[LogicalConnectionConfig]
        case "parameters" => c.as[ParametersConnectionConfig]
        case "constant" => c.as[ConstantConnectionConfig]
        case _ => Left(io.circe.DecodingFailure(s"Unknown connection type: $connType", c.history))
      }
    } yield result
  }
}

// Represents a bus connection in the YAML
case class BusConnectionConfig(
  connection: String,
  `type`: String,
  bus_name: String,
  bus_width: Int,
  bus_protocol: String,

  supplier_bus_name: String,
  consumer_bus_name: String,
  silent: Boolean
) extends ConnectionConfig

object BusConnectionConfig {
  import CirceDefaults._

  implicit val decoder: Decoder[BusConnectionConfig] = 
    Decoder.instance { d =>
      for {
        c <- d.downField("connection").as[String]
        t <- d.downField("type").as[String]
        bn <- withDefault(d, "bus_name", "")
        bw <- withDefault(d, "bus_width", 32)
        bp <- withDefault(d, "bus_protocol", "internal")
        sn <- withDefault(d, "supplier_bus_name", bn)
        cn <- withDefault(d, "consumer_bus_name", bn)
        ss <- withDefault(d, "silent", false)
      } yield BusConnectionConfig(c, t, bn, bw, bp, sn, cn, ss)
    }
}

// Represents a port connection in the YAML
case class PortConnectionConfig(
  connection: String,
  `type`: String
) extends ConnectionConfig

object PortConnectionConfig {
  implicit val decoder: Decoder[PortConnectionConfig] = deriveDecoder[PortConnectionConfig]
}

// Represents a port group connection in the YAML
case class PortGroupConnectionConfig(
  connection: String,
  `type`: String,
  first_prefix: Option[String] = None,
  second_prefix: Option[String] = None,
  excludes: Option[List[String]] = None
) extends ConnectionConfig

object PortGroupConnectionConfig {
  implicit val decoder: Decoder[PortGroupConnectionConfig] = deriveDecoder[PortGroupConnectionConfig]
}

// Represents a clock connection in the YAML
case class ClockConnectionConfig(
  connection: String,
  `type`: String
) extends ConnectionConfig

object ClockConnectionConfig {
  implicit val decoder: Decoder[ClockConnectionConfig] = deriveDecoder[ClockConnectionConfig]
}

// Represents a logical connection in the YAML
case class LogicalConnectionConfig(
  connection: String,
  `type`: String
) extends ConnectionConfig

object LogicalConnectionConfig {
  implicit val decoder: Decoder[LogicalConnectionConfig] = deriveDecoder[LogicalConnectionConfig]
}

// Represents a parameters connection in the YAML
case class ParametersConnectionConfig(
  connection: String,
  `type`: String,
  parameters: List[ParameterConfig]
) extends ConnectionConfig

object ParametersConnectionConfig {
  implicit val decoder: Decoder[ParametersConnectionConfig] = deriveDecoder[ParametersConnectionConfig]
}

// Represents a parameter in a parameters connection
case class ParameterConfig(
  name: String,
  value: Json,
  `type`: Option[String] = None
)

object ParameterConfig {
  implicit val decoder: Decoder[ParameterConfig] = deriveDecoder[ParameterConfig]
}

// Represents a constant connection in the YAML
case class ConstantConnectionConfig(
  connection: String,
  `type`: String,
  value: Option[Json] = None
) extends ConnectionConfig

object ConstantConnectionConfig {
  implicit val decoder: Decoder[ConstantConnectionConfig] = deriveDecoder[ConstantConnectionConfig]
}

// Represents a resource in a prefab
case class PrefabResourceConfig(
  resource: String
)
object PrefabResourceConfig {
  implicit val decoder: Decoder[PrefabResourceConfig] = deriveDecoder[PrefabResourceConfig]
}

// Represents an included prefab in a prefab
case class PrefabIncludeConfig(
  resource: String
)
object PrefabIncludeConfig {
  implicit val decoder: Decoder[PrefabIncludeConfig] = deriveDecoder[PrefabIncludeConfig]
}

// Represents a single prefab reference in the project file
case class PrefabConfig(
  name: String
)
object PrefabConfig {
  implicit val decoder: Decoder[PrefabConfig] = deriveDecoder[PrefabConfig]
}

case class SourceConfig(
  `type`: String, // Use backticks for type as it's a Scala keyword
  url: Option[String] = None, // For git and fetch types
  path: Option[String] = None // For local type
)

object SourceConfig {
  implicit val decoder: Decoder[SourceConfig] = deriveDecoder[SourceConfig]
}

case class InfoConfig(
  name: String = "",
  version: Option[String] = None,
  author: Option[String] = None,
  description: Option[String] = None
)
object InfoConfig {
  implicit val decoder: Decoder[InfoConfig] = deriveDecoder[InfoConfig]
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
  implicit val decoder: Decoder[CatalogFileConfig] = deriveDecoder[CatalogFileConfig]
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

// Represents the top-level structure of a prefab file
case class PrefabFileConfig(
  resources: Option[List[String]] = None,
  include: Option[List[PrefabIncludeConfig]] = None,
  instance: Option[List[InstanceConfig]] = None,
  connection: Option[List[ConnectionConfig]] = None
)
object PrefabFileConfig {
  import CustomDecoders._
  implicit val decoder: Decoder[PrefabFileConfig] = deriveDecoder[PrefabFileConfig]
}

// Represents a single definition within a file
case class DefinitionConfig(
  name: String,
  `type`: String, // Use backticks for type as it's a Scala keyword
  config: Map[String, Any] = Map.empty // Flexible for various config types
  // TODO: Add other definition fields if necessary based on Definition.scala
)
object DefinitionConfig {
  import CustomDecoders._
  implicit val decoder: Decoder[DefinitionConfig] = deriveDecoder[DefinitionConfig]
}
