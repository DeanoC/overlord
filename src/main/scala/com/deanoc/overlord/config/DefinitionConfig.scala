package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor}
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import com.deanoc.overlord.config.CirceDefaults.withDefault
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant

// Represents a single definition within a file
sealed trait DefinitionConfig {
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
  val gateware: Option[SourceConfig]
  val ports: Option[List[String]]
  val registers: Option[List[SourceConfig]]
  val drivers: Option[List[String]]
  val max_instances: Option[Int]
  val supplier_prefix: Option[String]
  val width: Option[Int]
} 

sealed trait SoftwareDefinitionConfig extends DefinitionConfig {
}

case class RamDefinitionConfig(
  `type`: String,
  ranges: List[MemoryRangeConfig],
  gateware: Option[SourceConfig] = None,
  ports: Option[List[String]] = None,
  registers: Option[List[SourceConfig]] = None,
  drivers: Option[List[String]] = None,
  max_instances: Option[Int] = None,
  supplier_prefix: Option[String] = None,
  width: Option[Int] = None,
  attributes: Map[String, Any] = Map.empty
) extends HardwareDefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object RamDefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[RamDefinitionConfig] = (c: HCursor) => {
    for {
      typeVal <- c.downField("type").as[String]
      ranges <- c.downField("ranges").as[List[MemoryRangeConfig]]
      ports = c.downField("ports").as[Option[List[String]]].toOption.flatten
      registers = c.downField("registers").as[Option[List[SourceConfig]]].toOption.flatten
      drivers = c.downField("drivers").as[Option[List[String]]].toOption.flatten
      maxInstances = c.downField("max_instances").as[Option[Int]].toOption.flatten
      gateware = c.downField("gateware").as[SourceConfig].toOption
      supplierPrefix = c.downField("supplier_prefix").as[Option[String]].toOption.flatten
      width = c.downField("width").as[Option[Int]].toOption.flatten
      attributes = ReflectionHelper.extractUnhandledFields[RamDefinitionConfig](c)
    } yield RamDefinitionConfig(typeVal, ranges, gateware, ports, registers, drivers, maxInstances, supplierPrefix, width, attributes)
  }
}

case class CpuDefinitionConfig(
  `type`: String,
  triple: String,
  core_count: Int = 1,
  gateware: Option[SourceConfig] = None,
  ports: Option[List[String]] = None,
  registers: Option[List[SourceConfig]] = None,
  drivers: Option[List[String]] = None,
  max_instances: Option[Int] = None,
  supplier_prefix: Option[String] = None,
  width: Option[Int] = None,
  attributes: Map[String, Any] = Map.empty
) extends HardwareDefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object CpuDefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[CpuDefinitionConfig] = (c: HCursor) => {
    for {
      typeVal <- c.downField("type").as[String]
      triple <- c.downField("triple").as[String]
      coreCount <- withDefault(c, "core_count", 1)
      gateware = c.downField("gateware").as[SourceConfig].toOption
      ports = c.downField("ports").as[Option[List[String]]].toOption.flatten
      registers = c.downField("registers").as[Option[List[SourceConfig]]].toOption.flatten
      drivers = c.downField("drivers").as[Option[List[String]]].toOption.flatten
      maxInstances = c.downField("max_instances").as[Option[Int]].toOption.flatten
      supplierPrefix = c.downField("supplier_prefix").as[Option[String]].toOption.flatten
      width = c.downField("width").as[Option[Int]].toOption.flatten
      attributes = ReflectionHelper.extractUnhandledFields[CpuDefinitionConfig](c)
    } yield CpuDefinitionConfig(typeVal, triple, coreCount, gateware, ports, registers, drivers, maxInstances, supplierPrefix, width, attributes)
  }
}

case class OtherDefinitionConfig(
  `type`: String,
  gateware: Option[SourceConfig] = None,
  ports: Option[List[String]] = None,
  registers: Option[List[SourceConfig]] = None,
  drivers: Option[List[String]] = None,
  max_instances: Option[Int] = None,
  supplier_prefix: Option[String] = None,
  width: Option[Int] = None,
  attributes: Map[String, Any] = Map.empty
) extends HardwareDefinitionConfig {
  def withAttributes(newAttributes: Map[String, Any]): DefinitionConfig = 
    copy(attributes = newAttributes)
}

object OtherDefinitionConfig {
  import CustomDecoders._
  
  implicit val decoder: Decoder[OtherDefinitionConfig] = (c: HCursor) => {
    for {
      typeVal <- c.downField("type").as[String]
      gateware = c.downField("gateware").as[SourceConfig].toOption
      ports = c.downField("ports").as[Option[List[String]]].toOption.flatten
      registers = c.downField("registers").as[Option[List[SourceConfig]]].toOption.flatten
      drivers = c.downField("drivers").as[Option[List[String]]].toOption.flatten
      maxInstances = c.downField("max_instances").as[Option[Int]].toOption.flatten
      supplierPrefix = c.downField("supplier_prefix").as[Option[String]].toOption.flatten
      width = c.downField("width").as[Option[Int]].toOption.flatten
      attributes = ReflectionHelper.extractUnhandledFields[OtherDefinitionConfig](c)
    } yield OtherDefinitionConfig(typeVal, gateware, ports, registers, drivers, maxInstances, supplierPrefix, width, attributes)
  }
}
