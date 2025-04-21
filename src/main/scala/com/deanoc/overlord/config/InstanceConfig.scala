package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor}
import io.circe.generic.semiauto._
import com.deanoc.overlord.utils.Utils.ReflectionHelper

// Represents a single instance in the project file
sealed trait  InstanceConfig {
  def name: String
  def `type`: String // Use backticks for type as it's a Scala keyword
  def attributes: Map[String, Any]
}

object InstanceConfig {
    import CustomDecoders._
  
    implicit val decoder: Decoder[InstanceConfig] = (c: HCursor) => {
      for {
        ft <- c.downField("type").as[String]
        typeField <- Right(ft.split('.').headOption.getOrElse(""))
        result <- typeField match {
          case _ => c.as[OtherInstanceConfig]
        }
      } yield result
    }}


case class OtherInstanceConfig(
  name: String,
  `type`: String,
  attributes: Map[String, Any] = Map.empty
) extends InstanceConfig

object OtherInstanceConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[OtherInstanceConfig] = (c: HCursor) => {
    for {
      name <- c.downField("name").as[String]
      typeVal <- c.downField("type").as[String]
      // Extract any additional fields not explicitly defined in the case class
      attributes = ReflectionHelper.extractUnhandledFields[OtherInstanceConfig](c)
    } yield OtherInstanceConfig(name, typeVal, attributes)
  }
}
// Add other specific instance configurations here as needed