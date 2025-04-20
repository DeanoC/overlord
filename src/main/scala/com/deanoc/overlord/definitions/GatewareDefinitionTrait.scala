package com.deanoc.overlord.definitions

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.Variant

trait GatewareDefinitionTrait extends ChipDefinitionTrait {
  val actionsFile: ActionsFile
  val parameters: Map[String, Variant]
}
