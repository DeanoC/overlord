package com.deanoc.overlord.definitions

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.config.GatewareConfig

trait GatewareDefinitionTrait extends ChipDefinitionTrait {
  val gatewareConfig: GatewareConfig
 // val actionsFile: ActionsFile
 // val parameters: Map[String, Variant]
} 
