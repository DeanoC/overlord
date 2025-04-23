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

  /**
   * Checks Defaults stack for a value for the given field.
   * If present, uses it (with type conversion), otherwise falls back to JSON.
   */
  def withDefaultsOverride[T](cursor: HCursor, field: String)(implicit decoder: Decoder[T]): Decoder.Result[T] = {
    if (Defaults.contains(field)) {
      // Try to convert the Any to T using circe's Decoder
      val value = Defaults(field)
      // Convert value to Json, then decode as T
      io.circe.parser.parse(io.circe.Printer.noSpaces.print(io.circe.Json.fromString(value.toString))).getOrElse(io.circe.Json.Null).as[T] match {
        case Right(v) => Right(v)
        case Left(_) =>
          // Fallback: try to cast directly if possible
          try Right(value.asInstanceOf[T])
          catch { case _: Throwable => Left(DecodingFailure(s"Failed to convert default for $field", cursor.history)) }
      }
    } else {
      cursor.downField(field).as[T]
    }
  }
}

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

  /**
   * Decoder for a field called "defaults" that decodes all key-value pairs into a Map[String, Any].
   * Only allows primitive values (String, Boolean, Number). Objects/arrays cause decoding failure.
   * When decoded, pushes the map onto the Defaults stack.
   */
  val decodeDefaultsAsMap: Decoder[Map[String, Any]] = Decoder.instance { c =>
    c.downField("defaults").as[Option[Map[String, Json]]].flatMap {
      case Some(map) =>
        val result = map.foldLeft(Right(Map.empty[String, Any]): Decoder.Result[Map[String, Any]]) {
          case (acc, (k, v)) =>
            acc.flatMap { m =>
              if (v.isString) Right(m + (k -> v.asString.get))
              else if (v.isBoolean) Right(m + (k -> v.asBoolean.get))
              else if (v.isNumber) Right(m + (k -> v.asNumber.flatMap(_.toBigDecimal).get))
              else Left(DecodingFailure(s"defaults field '$k' must be a primitive (string, boolean, number), got: $v", c.history))
            }
        }
        result.foreach(Defaults.push)
        result
      case None =>
        Defaults.push(Map.empty) // Always push an empty map if no defaults block
        Right(Map.empty[String, Any])
    }
  }
}
// Example usage for a config class with a defaults block
// case class ExampleConfig(defaults: Map[String, Any], other: String)

// object ExampleConfig {
//   import CustomDecoders._
//   implicit val decoder: Decoder[ExampleConfig] = Decoder.instance { c =>
//     for {
//       defaults <- decodeDefaultsAsMap(c)
//       other <- c.downField("other").as[String]
//     } yield {
//       try {
//         ExampleConfig(defaults, other)
//       } finally {
//         Defaults.pop()
//       }
//     }
//   }
// }


sealed case class BitsDesc(text: String) {
  private val txt: String = text.filterNot(c => c == '[' || c == ']')
  private val singleDigit: Boolean = txt.split(":").length < 2

  lazy val mask: Long = ((1L << bitCount.toLong) - 1L) << lo.toLong
  val hi: Int = Integer.parseInt(txt.split(":")(0))
  val lo: Int =
    if (singleDigit) hi
    else Integer.parseInt(txt.split(":")(1))
  val bitCount: Int = (hi - lo) + 1
  val singleBit: Boolean = singleDigit || bitCount == 1
}

object BitsDesc {
  def apply(width: Int): BitsDesc = BitsDesc(s"[${width - 1}:0]")
}

// Represents a memory range within a RAM definition
case class MemoryRangeConfig(
  address: String, // Assuming address is represented as a hex string in YAML
  size: String     // Assuming size is represented as a hex string in YAML
) derives Decoder

case class BoundraryConfig(
  name: String,
  width: String,
  direction: WireDirection = WireDirection.InOut,
  bitsDesc: BitsDesc = null
)

object BoundraryConfig {
  
  implicit val decoder: Decoder[BoundraryConfig] = (c: HCursor) => {
    for {
      name <- c.downField("name").as[String]
      width <- c.downField("width").as[String]
      dirStr <- withDefault(c, "direction", "inout")
      wireDir = WireDirection(dirStr)
      bitsDesc = try {
        BitsDesc(width)
      } catch {
        case e: Exception => 
          println(s"Failed to parse width '$width': ${e.getMessage}")
          BitsDesc(1)
      }
    } yield BoundraryConfig(name, width, wireDir, bitsDesc)
  }
}

case class InfoConfig(
  name: String = "",
  version: Option[String] = None,
  author: Option[String] = None,
  description: Option[String] = None
) derives Decoder

// configuration specific to gateware
case class GatewareConfig(
  actions: List[ActionConfig] = List.empty,
  parameters: Map[String, Any] = Map.empty,
)
 
object GatewareConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[GatewareConfig] = new Decoder[GatewareConfig] {
    def apply(c: HCursor): Decoder.Result[GatewareConfig] = {
      for {
        actions <- c.downField("actions").as[List[ActionConfig]]
        parameters <- withDefault(c, "parameters", Map.empty)
      } yield GatewareConfig(actions, parameters)
    }
  }
}

object DefaultsAwareDecoders {
  import io.circe.Decoder
  import io.circe.CursorOp

  private def fieldNameFromCursor(c: HCursor): Option[String] =
    c.history.collectFirst { case CursorOp.DownField(field) => field }

  implicit val stringDecoder: Decoder[String] = Decoder.instance { c =>
    val field = fieldNameFromCursor(c).getOrElse("")
    if (Defaults.contains(field)) CirceDefaults.withDefaultsOverride[String](c, field)
    else Decoder.decodeString(c)
  }

  implicit val intDecoder: Decoder[Int] = Decoder.instance { c =>
    val field = fieldNameFromCursor(c).getOrElse("")
    if (Defaults.contains(field)) CirceDefaults.withDefaultsOverride[Int](c, field)
    else Decoder.decodeInt(c)
  }

  implicit val booleanDecoder: Decoder[Boolean] = Decoder.instance { c =>
    val field = fieldNameFromCursor(c).getOrElse("")
    if (Defaults.contains(field)) CirceDefaults.withDefaultsOverride[Boolean](c, field)
    else Decoder.decodeBoolean(c)
  }

  implicit val doubleDecoder: Decoder[Double] = Decoder.instance { c =>
    val field = fieldNameFromCursor(c).getOrElse("")
    if (Defaults.contains(field)) CirceDefaults.withDefaultsOverride[Double](c, field)
    else Decoder.decodeDouble(c)
  }

  // Add more for other basic types as needed (Long, Float, etc.)
}

// To activate these globally, import DefaultsAwareDecoders._ in your config package or relevant files.
