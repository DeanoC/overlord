package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor}
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import com.deanoc.overlord.config.CirceDefaults.withDefault
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.config._

// Represents a single definition within a file
trait DefinitionConfig {
  def `type`: String // Use backticks for type as it's a Scala keyword
  def attributes: Map[String, Any]
  
  // Add this method to allow creating a new instance with updated attributes
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig

  def attributesAsVariant: Map[String, Variant] = {
    attributes.map { case (k, v) =>
      k -> Utils.toVariant(v) // Convert Any to Variant
    }
  }
}

object DefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[DefinitionConfig] = (c: HCursor) => {
    for {
      ft <- c.downField("type").as[String]
      typeField <- Right(ft.split('.').headOption.getOrElse(""))
      result <- typeField match {
        case "ram" => c.as[RamDefinitionConfig]
        case "cpu" => c.as[CpuDefinitionConfig]
        case _ => c.as[OtherDefinitionConfig]
      }
    } yield result
  }
}

sealed trait SoftwareDefinitionConfig extends DefinitionConfig {}