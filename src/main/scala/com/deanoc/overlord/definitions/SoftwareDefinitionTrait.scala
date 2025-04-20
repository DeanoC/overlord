package com.deanoc.overlord.definitions

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.instances._
import com.deanoc.overlord.config._
import io.circe.parser.decode

import java.nio.file.Path

trait SoftwareDefinitionTrait extends DefinitionTrait {
  val actionsFilePath: Path
  val actionsFile: ActionsFile
  val parameters: Map[String, Variant]

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Option[Map[String, Any]]
  ): Either[String, InstanceTrait] = {
    defType match {
      case _: DefinitionType.LibraryDefinition =>
        instanceConfig match {
          case Some(configMap) =>
            decode[LibraryConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match {
              case Right(libraryConfig) =>
                LibraryInstance(name, this, libraryConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]]
              case Left(error) =>
                Left(
                  s"Failed to decode LibraryConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"Library definition instance $name requires configuration")
        }
      case _: DefinitionType.ProgramDefinition =>
        instanceConfig match {
          case Some(configMap) =>
            decode[ProgramConfig](
              Definition.anyToJson(configMap).noSpaces
            ) match {
              case Right(programConfig) =>
                ProgramInstance(name, this, programConfig).asInstanceOf[Either[
                  String,
                  InstanceTrait
                ]]
              case Left(error) =>
                Left(
                  s"Failed to decode ProgramConfig for instance $name: ${error.getMessage}"
                )
            }
          case None =>
            Left(s"Program definition instance $name requires configuration")
        }
      case _ => Left(s"$defType is invalid for software")
    }
  }
}
