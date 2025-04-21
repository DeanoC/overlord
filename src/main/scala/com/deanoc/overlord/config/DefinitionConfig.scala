package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor}
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import com.deanoc.overlord.config.CirceDefaults.withDefault
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant

// Represents a single definition within a file
sealed trait DefinitionConfig {
  def name: String
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

sealed trait HardwareDefinitionConfig extends DefinitionConfig {
  val gatewareConfig: Option[SourceConfig]
} 

sealed trait SoftwareDefinitionConfig extends DefinitionConfig {
}

case class RamDefinitionConfig(
  name: String,
  `type`: String,
  ranges: List[MemoryRangeConfig],
  gatewareConfig: Option[SourceConfig] = None,
  attributes: Map[String, Any] = Map.empty
) extends HardwareDefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object RamDefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[RamDefinitionConfig] = (c: HCursor) => {
    for {
      name <- c.downField("name").as[String]
      typeVal <- c.downField("type").as[String]
      ranges <- c.downField("ranges").as[List[MemoryRangeConfig]]
      gatewareConfig = c.downField("gateware").as[SourceConfig].toOption
      attributes = ReflectionHelper.extractUnhandledFields[RamDefinitionConfig](c)
    } yield RamDefinitionConfig(name, typeVal, ranges, gatewareConfig, attributes)
  }
}

case class CpuDefinitionConfig(
  name: String,
  `type`: String,
  triple: String,
  core_count: Int = 1,
  gatewareConfig: Option[SourceConfig] = None,
  attributes: Map[String, Any] = Map.empty
) extends HardwareDefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object CpuDefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[CpuDefinitionConfig] = (c: HCursor) => {
    for {
      name <- c.downField("name").as[String]
      typeVal <- c.downField("type").as[String]
      triple <- c.downField("triple").as[String]
      coreCount <- withDefault(c, "core_count", 1)
      gatewareConfig = c.downField("gateware").as[SourceConfig].toOption
      attributes = ReflectionHelper.extractUnhandledFields[CpuDefinitionConfig](c)
    } yield CpuDefinitionConfig(name, typeVal, triple, coreCount, gatewareConfig, attributes)
  }
}

case class OtherDefinitionConfig(
  name: String,
  `type`: String,
  gatewareConfig: Option[SourceConfig] = None,
  attributes: Map[String, Any] = Map.empty
) extends HardwareDefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object OtherDefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[OtherDefinitionConfig] = (c: HCursor) => {
    for {
      name <- c.downField("name").as[String]
      typeVal <- c.downField("type").as[String]
      gatewareConfig = c.downField("gateware").as[SourceConfig].toOption
      attributes = ReflectionHelper.extractUnhandledFields[OtherDefinitionConfig](c)
    } yield OtherDefinitionConfig(name, typeVal, gatewareConfig, attributes)
  }
}
