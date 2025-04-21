package com.deanoc.overlord.definitions

import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.instances._
import com.deanoc.overlord.config._

import java.nio.file.Path

trait DefinitionTrait {
  val defType: DefinitionType
  val config: DefinitionConfig
  val sourcePath: Path
  val dependencies: Seq[String]

  // Modified to accept Option[Map[String, Any]] for instance-specific config
  def createInstance(
      name: String,
      instanceConfig: Map[String, Any]
  ): Either[String, InstanceTrait]
}
