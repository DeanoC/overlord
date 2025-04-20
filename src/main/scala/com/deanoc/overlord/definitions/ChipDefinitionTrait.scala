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
      instanceConfig: Option[Map[String, Any]]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: RamDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            // Attempt to decode the config map into RamConfig
            decode[RamConfig](
              Definition.anyToJson(configMap).noSpaces
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
          case None =>
            Left(s"RamDefinitionType instance $name requires configuration")
        }
      case _: CpuDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            decode[CpuConfig](
              Definition.anyToJson(configMap).noSpaces
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
          case None =>
            Left(s"CpuDefinitionType instance $name requires configuration")
        }
      case _: IoDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            decode[IoConfig](
              Definition.anyToJson(configMap).noSpaces
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
          case None =>
            Left(s"IoDefinitionType instance $name requires configuration")
        }
      case _: PinGroupDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            decode[PinGroupConfig](
              Definition.anyToJson(configMap).noSpaces
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
          case None =>
            Left(
              s"PinGroupDefinitionType instance $name requires configuration"
            )
        }
      case _: ClockDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            decode[ClockConfig](
              Definition.anyToJson(configMap).noSpaces
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
          case None =>
            Left(s"ClockDefinitionType instance $name requires configuration")
        }
      case _: BoardDefinitionType =>
        instanceConfig match {
          case Some(configMap) =>
            decode[BoardConfig](
              Definition.anyToJson(configMap).noSpaces
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
          case None =>
            Left(s"BoardDefinitionType instance $name requires configuration")
        }
      case _: GraphicDefinitionType => {
        val graphic = GraphicInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        graphic.mergeAllAttributes(attribs)
        Right(graphic)
      }
      case _: StorageDefinitionType => {
        val storage = StorageInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        storage.mergeAllAttributes(attribs)
        Right(storage)
      }
      case _: NetDefinitionType => {
        val net = NetInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        net.mergeAllAttributes(attribs)
        Right(net)
      }
      case _: SocDefinitionType => {
        val soc = SocInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        soc.mergeAllAttributes(attribs)
        Right(soc)
      }
      case _: SwitchDefinitionType => {
        val sw = SwitchInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        sw.mergeAllAttributes(attribs)
        Right(sw)
      }
      case _: OtherDefinitionType => {
        val other = OtherInstance(name, this)
        val attribs = instanceConfig.getOrElse(Map[String, Any]()).map {
          case (k, v) => k -> com.deanoc.overlord.utils.Utils.toVariant(v)
        }
        other.mergeAllAttributes(attribs)
        Right(other)
      }
      case _ => Left(s"$defType is invalid for chip")
    }
  }
}
