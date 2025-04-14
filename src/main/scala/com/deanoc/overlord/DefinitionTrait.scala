package com.deanoc.overlord

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.hardware.{
  HardwareDefinition,
  Port,
  RegisterBank,
  Registers
}
import com.deanoc.overlord.instances._
import com.deanoc.overlord.software.SoftwareDefinition
import com.deanoc.overlord.Project

import java.nio.file.Path
import scala.sys.exit

trait DefinitionTrait {
  val defType: DefinitionType
  val attributes: Map[String, Variant]
  val sourcePath: Path
  val dependencies: Seq[String]

  def createInstance(
      name: String,
      attribs: Map[String, Variant]
  ): Either[String, InstanceTrait]
}

object Definition {
  def apply(defi: Variant, defaults: Map[String, Variant]): DefinitionTrait = {

    val path = Project.catalogPath

    val table = Utils.mergeAintoB(Utils.toTable(defi), defaults)
    val deftype = DefinitionType(Utils.toString(table("type"))) match {
      case _: ChipDefinitionType     => HardwareDefinition(table, path)
      case _: SoftwareDefinitionType => SoftwareDefinition(table, path)

      // TODO these aren't really chips, but piggyback off HardwareDefinition for now
      case _: PinGroupDefinitionType => HardwareDefinition(table, path)
      case _: ClockDefinitionType    => HardwareDefinition(table, path)
      case _: BoardDefinitionType    => HardwareDefinition(table, path)
    }

    deftype match {
      case Right(value) => value
      case Left(error) =>
        println(s"Invalid definition type $path: $error")
        exit()
      // The following cases should no longer be needed once all methods return Either
      case Some(value) =>
        value // For backward compatibility with methods still returning Option
      case None =>
        println(s"Invalid definition type $path")
        exit()
    }
  }
}

trait ChipDefinitionTrait extends DefinitionTrait {
  val ports: Map[String, Port]
  val maxInstances: Int
  val defType: DefinitionType
  val attributes: Map[String, Variant]
  protected val registersV: Seq[Variant]
  var registers: Seq[RegisterBank] = Seq()

  def createInstance(
      name: String,
      attribs: Map[String, Variant]
  ): Either[String, InstanceTrait] = {
    val instanceResult = defType match {
      case _: RamDefinitionType      => RamInstance(name, this, attribs)
      case _: CpuDefinitionType      => CpuInstance(name, this, attribs)
      case _: GraphicDefinitionType  => GraphicInstance(name, this, attribs)
      case _: StorageDefinitionType  => StorageInstance(name, this, attribs)
      case _: NetDefinitionType      => NetInstance(name, this, attribs)
      case _: IoDefinitionType       => IoInstance(name, this, attribs)
      case _: SocDefinitionType      => SocInstance(name, this, attribs)
      case _: SwitchDefinitionType   => SwitchInstance(name, this, attribs)
      case _: OtherDefinitionType    => OtherInstance(name, this, attribs)
      case _: PinGroupDefinitionType => PinGroupInstance(name, this, attribs)
      case _: ClockDefinitionType    => ClockInstance(name, this, attribs)
      case _: BoardDefinitionType    => BoardInstance(name, this, attribs)
      case _                         => Left(s"$defType is invalid for chip")
    }

    instanceResult match {
      case Right(instance) =>
        registers = Registers(instance, registersV)
        Right(instance)
      case Left(error) => Left(error)
    }
  }
}

trait GatewareDefinitionTrait extends ChipDefinitionTrait {
  val actionsFile: ActionsFile
  val parameters: Map[String, Variant]
}

trait SoftwareDefinitionTrait extends DefinitionTrait {
  val actionsFilePath: Path
  val actionsFile: ActionsFile
  val parameters: Map[String, Variant]

  def createInstance(
      name: String,
      attribs: Map[String, Variant]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: LibraryDefinitionType => LibraryInstance(name, this, attribs)
      case _: ProgramDefinitionType => ProgramInstance(name, this, attribs)
      case _                        => Left(s"$defType is invalid for software")
    }
  }
}

trait HardwareDefinitionTrait extends ChipDefinitionTrait
