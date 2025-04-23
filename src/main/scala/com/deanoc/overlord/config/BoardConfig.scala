package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor, DecodingFailure, Json}
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import com.deanoc.overlord.config.CirceDefaults.withDefault

// Trait for common IO config fields
trait BoardIOConfig {
  def drive: String
  def pullup: String
  def slew: String
  def standard: String
}

object BoardIOConfig {
  // Shared decoder for the common fields
  def decodeCommonFields(c: HCursor): Decoder.Result[(String, String, String, String)] = for {
    drive <- c.downField("drive").as[String]
    pullup <- c.downField("pullup").as[String]
    slew <- c.downField("slew").as[String]
    standard <- c.downField("standard").as[String]
  } yield (drive, pullup, slew, standard)
}

// Represents a clock within a Board definition
case class BoardClockConfig(
  name: String,
  frequency: String, // Assuming frequency is a string like "100MHz"
  drive: String,
  pullup: String,
  slew: String,
  standard: String
) extends BoardIOConfig

object BoardClockConfig {
  implicit val decoder: Decoder[BoardClockConfig] = (c: HCursor) =>
    BoardIOConfig.decodeCommonFields(c).flatMap { case (drive, pullup, slew, standard) =>
      for {
        name <- c.downField("name").as[String]
        frequency <- c.downField("frequency").as[String]
      } yield BoardClockConfig(name, frequency, drive, pullup, slew, standard)
    }
}

case class PingroupsConfig(
  names: List[String],
  pins: List[String],
  drive: String,
  pullup: String,
  slew: String,
  standard: String
) extends BoardIOConfig

object PingroupsConfig {
  implicit val decoder: Decoder[PingroupsConfig] = (c: HCursor) => {
    def stringOrList(fieldPlural: String, fieldSingular: String): Decoder.Result[List[String]] =
      c.downField(fieldPlural).focus match {
        case Some(json) if json.isString =>
          c.downField(fieldPlural).as[String].map(List(_))
        case Some(json) if json.isArray =>
          c.downField(fieldPlural).as[List[String]]
        case None =>
          // Try singular
          c.downField(fieldSingular).focus match {
            case Some(json) if json.isString =>
              c.downField(fieldSingular).as[String].map(List(_))
            case Some(json) if json.isArray =>
              c.downField(fieldSingular).as[List[String]]
            case Some(_) =>
              Left(DecodingFailure(s"$fieldSingular must be string or list of strings", c.downField(fieldSingular).history))
            case None =>
              Left(DecodingFailure(s"Missing field: $fieldPlural or $fieldSingular", c.history))
          }
        case Some(_) =>
          Left(DecodingFailure(s"$fieldPlural must be string or list of strings", c.downField(fieldPlural).history))
      }

    for {
      names <- stringOrList("names", "name")
      pins <- stringOrList("pins", "pin")
      _ <- if (names.length == pins.length) Right(()) else
        Left(DecodingFailure("names and pins must have the same length", c.history))
    } yield (names, pins)
  }.flatMap { case (names, pins) =>
    BoardIOConfig.decodeCommonFields(c).map { case (drive, pullup, slew, standard) =>
      PingroupsConfig(names, pins, drive, pullup, slew, standard)
    }
  }
}

case class BoardDefinitionConfig(
  `type`: String,
  board_type: String,
  board_family: String,
  board_device: String,

  clocks: List[BoardClockConfig],
  pingroups: List[PingroupsConfig],

  attributes: Map[String, Any] = Map.empty
) extends DefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object BoardDefinitionConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[BoardDefinitionConfig] = (c: HCursor) => {
    for {
      defaults <- decodeDefaultsAsMap(c)
      typeVal <- c.downField("type").as[String]
      boardType <- c.downField("board_type").as[String]
      boardFamily <- c.downField("board_family").as[String]
      boardDevice <- c.downField("board_device").as[String]
      clocks <- withDefault(c, "clocks", List.empty[BoardClockConfig])
      pingroups <- withDefault(c, "pingroups", List.empty[PingroupsConfig])
      attributes = ReflectionHelper.extractUnhandledFields[BoardDefinitionConfig](c)
    } yield {
      try {
        BoardDefinitionConfig(
          `type` = typeVal,
          board_type = boardType,
          board_family = boardFamily,
          board_device = boardDevice,
          clocks = clocks,
          pingroups = pingroups,
          attributes = attributes
        )
      } finally {
        Defaults.pop()
      }
    }
  }
}
