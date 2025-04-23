package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor}
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import com.deanoc.overlord.config.CirceDefaults.withDefault
import com.deanoc.overlord.utils.Utils
import com.deanoc.overlord.utils.Variant

sealed trait HardwareDefinitionConfig extends DefinitionConfig {
  val gateware: Option[SourceConfig]
  val boundraries: List[BoundraryConfig]
  val registers: List[SourceConfig]
  val drivers: List[String]
  val max_instances: Int
  val supplier_prefix: Option[List[String]]
  val width: Int
}

object HardwareDefinitionConfig {
  import CustomDecoders._

  // Helper method to decode common fields
  def decodeCommonFields(c: HCursor): Decoder.Result[(Option[SourceConfig], List[BoundraryConfig], List[SourceConfig], List[String], Int, Option[List[String]], Int, Map[String, Any])] = {
    for {
      gateware <- c.downField("gateware").as[Option[SourceConfig]]
      // Handle both singular "port" and plural "ports" fields
      ports <- {
        val hasBoundrary = c.downField("boundrary").succeeded
        val hasBoundraries = c.downField("boundraries").succeeded
        if (hasBoundrary && hasBoundraries) {
          Left(io.circe.DecodingFailure("Cannot have both 'boundrary' and 'boundraries' in the same definition", c.history))
        } else if (hasBoundrary) {
          c.downField("boundrary").as[BoundraryConfig].map(port => List(port))
        } else {
          withDefault(c, "boundraries", List.empty[BoundraryConfig])
        }
      }
      registers <- withDefault(c, "registers", List.empty[SourceConfig])
      drivers <- withDefault(c, "drivers", List.empty[String])
      maxInstances <- withDefault(c, "max_instances", 1)
      supplierPrefix <- c.downField("supplier_prefix").as[Option[List[String]]]
      width <- withDefault(c, "width", 32)
      attributes = ReflectionHelper.extractUnhandledFields[HardwareDefinitionConfig](c)
    } yield (gateware, ports, registers, drivers, maxInstances, supplierPrefix, width, attributes)
  }
}

case class RamDefinitionConfig(
  `type`: String,
  ranges: List[MemoryRangeConfig],
  gateware: Option[SourceConfig] = None,
  boundraries: List[BoundraryConfig] = List(),
  registers: List[SourceConfig] = List(),
  drivers: List[String] = List(),
  max_instances: Int = 1,
  supplier_prefix: Option[List[String]] = None,
  width: Int = 32,
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
      commonFields <- HardwareDefinitionConfig.decodeCommonFields(c)
    } yield RamDefinitionConfig(
      `type` = typeVal,
      ranges = ranges,
      gateware = commonFields._1,
      boundraries = commonFields._2,
      registers = commonFields._3,
      drivers = commonFields._4,
      max_instances = commonFields._5,
      supplier_prefix = commonFields._6,
      width = commonFields._7,
      attributes = commonFields._8
    )
  }
}

case class CpuDefinitionConfig(
  `type`: String,
  triple: String,
  core_count: Int = 1,
  gateware: Option[SourceConfig] = None,
  boundraries: List[BoundraryConfig] = List(),
  registers: List[SourceConfig] = List(),
  drivers: List[String] = List(),
  max_instances: Int = 1,
  supplier_prefix: Option[List[String]] = None,
  width: Int = 32,
  max_atomic_width: Int = 32, // Default same as width
  max_bitop_type_width: Int = 32,
  gcc_flags: String = "",
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
      width <- withDefault(c, "width", 32)
      maxAtomicWidth <- withDefault(c, "max_atomic_width", width) // Default to width
      maxBitOpTypeWidth <- withDefault(c, "max_bitop_type_width", 32)
      gccFlags <- withDefault(c, "gcc_flags", "")
      commonFields <- HardwareDefinitionConfig.decodeCommonFields(c)
    } yield CpuDefinitionConfig(
      `type` = typeVal,
      triple = triple,
      core_count = coreCount,
      gateware = commonFields._1,
      boundraries = commonFields._2,
      registers = commonFields._3,
      drivers = commonFields._4,
      max_instances = commonFields._5,
      supplier_prefix = commonFields._6,
      width = commonFields._7,
      max_atomic_width = maxAtomicWidth,
      max_bitop_type_width = maxBitOpTypeWidth,
      gcc_flags = gccFlags,
      attributes = commonFields._8
    )
  }
}

case class OtherDefinitionConfig(
  `type`: String,
  gateware: Option[SourceConfig] = None,
  boundraries: List[BoundraryConfig] = List(),
  registers: List[SourceConfig] = List(),
  drivers: List[String] = List(),
  max_instances: Int = 1,
  supplier_prefix: Option[List[String]] = None,
  width: Int = 32,
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
      commonFields <- HardwareDefinitionConfig.decodeCommonFields(c)
    } yield OtherDefinitionConfig(
      `type` = typeVal,
      gateware = commonFields._1,
      boundraries = commonFields._2,
      registers = commonFields._3,
      drivers = commonFields._4,
      max_instances = commonFields._5,
      supplier_prefix = commonFields._6,
      width = commonFields._7,
      attributes = commonFields._8
    )
  }
}
