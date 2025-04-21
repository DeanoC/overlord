package com.deanoc.overlord.definitions

import com.deanoc.overlord.hardware.{Port, RegisterBank}
import com.deanoc.overlord.instances._
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.config._
import io.circe.parser.decode

trait ChipDefinitionTrait extends DefinitionTrait {
  val ports: Map[String, Port]
  val maxInstances: Int
  val attributes: Map[String, Variant]
  protected val registersV: Seq[Variant]
  var registers: Seq[RegisterBank] = Seq()

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Map[String, Any]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: DefinitionType.RamDefinition =>
        // Attempt to decode the config map into RamConfig
        decode[RamConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(ramConfig) =>
            RamInstance(name, this, ramConfig).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode RamConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.CpuDefinition =>
        decode[CpuConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(cpuConfig) =>
            CpuInstance(name, this, cpuConfig).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode CpuConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.IoDefinition =>
        decode[IoConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(ioConfig) =>
            IoInstance(name, this, ioConfig).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode IoConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.PinGroupDefinition =>
        decode[PinGroupConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(pinGroupConfig) =>
            PinGroupInstance(
              name = name,
              definition = this,
              config = pinGroupConfig
            ).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode PinGroupConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.ClockDefinition =>
        decode[ClockConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(clockConfig) =>
            ClockInstance(name, this, clockConfig).asInstanceOf[Either[
              String,
              InstanceTrait
            ]]
          case Left(error) =>
            Left(
              s"Failed to decode ClockConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.BoardDefinition =>
        decode[BoardConfig](
          Definition.anyToJson(instanceConfig).noSpaces
        ) match {
          case Right(boardConfig) =>
            BoardInstance(
              name = name,
              definition = this,
              config = boardConfig
            )
          case Left(error) =>
            Left(
              s"Failed to decode BoardConfig for instance $name: ${error.getMessage}"
            )
        }
      case _: DefinitionType.GraphicDefinition => {
        val graphic = GraphicInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        graphic.mergeAllAttributes(attribs)
        Right(graphic)
      }
      case _: DefinitionType.StorageDefinition => {
        val storage = StorageInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        storage.mergeAllAttributes(attribs)
        Right(storage)
      }
      case _: DefinitionType.NetDefinition => {
        val net = NetInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        net.mergeAllAttributes(attribs)
        Right(net)
      }
      case _: DefinitionType.SocDefinition => {
        val soc = SocInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        soc.mergeAllAttributes(attribs)
        Right(soc)
      }
      case _: DefinitionType.SwitchDefinition => {
        val sw = SwitchInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        sw.mergeAllAttributes(attribs)
        Right(sw)
      }
      case _: DefinitionType.OtherDefinition => {
        val other = OtherInstance(name, this)
        val attribs = instanceConfig.map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        other.mergeAllAttributes(attribs)
        Right(other)
      }
      case _ => Left(s"$defType is invalid for chip")
    }
  }
}
