package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor}
import io.circe.generic.semiauto._
import com.deanoc.overlord.utils.Utils.ReflectionHelper

// Represents a single instance in the project file
sealed trait InstanceConfig {
  def name: String
  def `type`: String // Use backticks for type as it's a Scala keyword
  def attributes: Map[String, Any]
}

object InstanceConfig {
  import CustomDecoders._

  // Common decoder function that extracts shared fields for all instance configs
  def decodeCommonFields(c: HCursor): Decoder.Result[(String, String)] = {
    for {
      name <- c.downField("name").as[String]
      typeVal <- c.downField("type").as[String]
    } yield (name, typeVal)
  }

  implicit val decoder: Decoder[InstanceConfig] = (c: HCursor) => {
    for {
      ft <- c.downField("type").as[String]
      typeField <- Right(ft.split('.').headOption.getOrElse(""))
      result <- typeField match {
        case _ => c.as[OtherInstanceConfig]
      }
    } yield result
  }
}

case class OtherInstanceConfig(
    name: String,
    `type`: String,
    attributes: Map[String, Any] = Map.empty
) extends InstanceConfig

object OtherInstanceConfig {
  import CustomDecoders._

  implicit val decoder: Decoder[OtherInstanceConfig] = (c: HCursor) => {
    // First get the common fields
    InstanceConfig.decodeCommonFields(c).flatMap { case (name, typeVal) =>
      // Then extract additional fields
      val attributes = ReflectionHelper.extractUnhandledFields[OtherInstanceConfig](c)
      // Return the final result
      Right(OtherInstanceConfig(name, typeVal, attributes))
    }
  }
}
// Add other specific instance configurations here as needed
