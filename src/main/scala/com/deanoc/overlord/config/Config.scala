package com.deanoc.overlord.config

import io.circe.{Decoder, Json, HCursor}
import io.circe.generic.semiauto._
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import scala.collection.mutable
import com.deanoc.overlord.config.CirceDefaults.withDefault
import io.circe.DecodingFailure

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
  actions: List[String] = List.empty,
  parameters: Map[String, Any] = Map.empty,
)
 
object GatewareConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[GatewareConfig] = new Decoder[GatewareConfig] {
    def apply(c: HCursor): Decoder.Result[GatewareConfig] = {
      for {
        actions <- c.downField("actions").as[List[String]]
        parameters <- withDefault(c, "parameters", Map.empty)
      } yield GatewareConfig(actions, parameters)
    }
  }
}
