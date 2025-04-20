package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.definitions.ChipDefinitionTrait
import com.deanoc.overlord.interfaces.RamLike

import scala.reflect.ClassTag

case class RamInstance(
    name: String,
    override val definition: ChipDefinitionTrait,
    config: com.deanoc.overlord.config.RamConfig // Store the specific config
) extends ChipInstance
    with RamLike {
  private val cpuRegEx = "\\s*,\\s*".r

  private def decodeCpusString(cpus: String): Seq[String] =
    if (cpus == "_") Seq() else cpuRegEx.split(cpus).toSeq.map(_.toLowerCase())

  private lazy val ranges: Seq[(BigInt, BigInt, Boolean, Seq[String])] = {
    config.ranges.map { rangeConfig =>
      (
        BigInt(rangeConfig.address.stripPrefix("0x"), 16), // Convert hex string to BigInt
        BigInt(rangeConfig.size.stripPrefix("0x"), 16), // Convert hex string to BigInt
        false, // fixed_address is not in RamConfig, default to false
        Seq() // cpus is not in RamConfig, default to empty sequence
      )
    }
  }

  override def isVisibleToSoftware: Boolean = true // isVisibleToSoftware is not in RamConfig, keep default

  override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
    val RamLike_ = classOf[RamLike]
    tag.runtimeClass match {
      case RamLike_ => Some(asInstanceOf[T])
      case _        => super.getInterface[T](tag)
    }

  }

  override def getRanges: Seq[(BigInt, BigInt, Boolean, Seq[String])] = ranges
}

object RamInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: com.deanoc.overlord.config.RamConfig // Accept RamConfig
  ): Either[String, RamInstance] = {
    try {
      // Create the RamInstance, passing the config
      val ram = new RamInstance(name, definition, config)
      Right(ram)
    } catch {
      case e: Exception =>
        Left(s"Error creating RamInstance: ${e.getMessage}")
    }
  }
}
