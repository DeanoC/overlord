package com.deanoc.overlord.connections

import com.deanoc.overlord.definitions.{HardwareDefinition, GatewareDefinition, SoftwareDefinitionTrait}
import com.deanoc.overlord.instances.{PinGroupInstance, ClockInstance}

import com.deanoc.overlord.instances.InstanceTrait
import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.definitions.DefinitionTrait
import com.deanoc.overlord.definitions.GatewareDefinition

/** Represents a location in the connection system, combining an instance, an
  * optional port, and a fully qualified name.
  *
  * @param instance
  *   The hardware/software/gateware instance.
  * @param port
  *   Optional port if the connection involves a specific port.
  * @param fullName
  *   The fully qualified name of this location.
  */
case class InstanceLoc(
    instance: InstanceTrait,
    port: Option[HardwareBoundrary],
    fullName: String
) {

  /** Returns the definition associated with this instance. */
  def definition: DefinitionTrait = instance.definition

  /** Checks if this instance represents hardware. */
  val isHardware: Boolean = definition.isInstanceOf[HardwareDefinition]

  /** Checks if this instance represents gateware. */
  def isGateware: Boolean = definition.isInstanceOf[GatewareDefinition]

  /** Checks if this instance represents software. */
  def isSoftware: Boolean = definition.isInstanceOf[SoftwareDefinitionTrait]

  /** Checks if this instance is a pin group. */
  def isPin: Boolean = instance.isInstanceOf[PinGroupInstance]

  /** Checks if this instance is a clock. */
  def isClock: Boolean = instance.isInstanceOf[ClockInstance]

  /** Checks if this instance is a chip (not a pin or clock). */
  def isChip: Boolean = !(isPin || isClock)
}