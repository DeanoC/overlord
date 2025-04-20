package com.deanoc.overlord.definitions

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.instances._

import java.nio.file.Path

trait DefinitionTrait {
  val defType: DefinitionType
  val attributes: Map[String, Variant]
  val sourcePath: Path
  val dependencies: Seq[String]

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Option[Map[String, Any]]
  ): Either[String, InstanceTrait]
}
