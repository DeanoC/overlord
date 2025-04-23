package com.deanoc.overlord.config

import io.circe.Decoder
import io.circe.HCursor
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import com.deanoc.overlord.config.CirceDefaults.withDefault

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

case class BoardDefinitionConfig(
  `type`: String,
  ports: List[BoundraryConfig] = List(),
  attributes: Map[String, Any] = Map.empty
) extends DefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object BoardDefinitionConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[BoardDefinitionConfig] = (c: HCursor) => {
    for {
      typeVal <- c.downField("type").as[String]
      ports <- withDefault(c, "ports", List.empty[BoundraryConfig])
      attributes = ReflectionHelper.extractUnhandledFields[BoardDefinitionConfig](c)
    } yield BoardDefinitionConfig(
      `type` = typeVal,
      ports = ports,
      attributes = attributes
    )
  }
}
