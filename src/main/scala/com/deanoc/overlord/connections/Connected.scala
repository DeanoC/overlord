package com.deanoc.overlord.connections

import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.{
  BiDirectionConnection,
  ConnectionDirection,
  DefinitionTrait,
  GatewareDefinitionTrait,
  HardwareDefinitionTrait,
  QueryInterface,
  SoftwareDefinitionTrait
}
import com.deanoc.overlord.instances.{
  ChipInstance,
  ClockInstance,
  InstanceTrait,
  PinGroupInstance
}

/** Represents a location in the connection system, combining an instance, an
  * optional port, and a fully qualified name.
  * 
  * @param instance The hardware/software/gateware instance.
  * @param port Optional port if the connection involves a specific port.
  * @param fullName The fully qualified name of this location.
  */
case class InstanceLoc(
    instance: InstanceTrait,
    port: Option[Port],
    fullName: String
) {

  /** Returns the definition associated with this instance. */
  def definition: DefinitionTrait = instance.definition

  /** Checks if this instance represents hardware. */
  val isHardware: Boolean = definition.isInstanceOf[HardwareDefinitionTrait]

  /** Checks if this instance represents gateware. */
  def isGateware: Boolean = definition.isInstanceOf[GatewareDefinitionTrait]

  /** Checks if this instance represents software. */
  def isSoftware: Boolean = definition.isInstanceOf[SoftwareDefinitionTrait]

  /** Checks if this instance is a pin group. */
  def isPin: Boolean = instance.isInstanceOf[PinGroupInstance]

  /** Checks if this instance is a clock. */
  def isClock: Boolean = instance.isInstanceOf[ClockInstance]

  /** Checks if this instance is a chip (not a pin or clock). */
  def isChip: Boolean = !(isPin || isClock)
}

/** Base trait for all connection types in the system.
  * 
  * Connections represent links between components, such as buses, logical
  * connections, or constants. These are defined in configuration files like
  * `fabric.yaml` with fields such as `type`, `connection`, and optional
  * `bus_name`.
  */
trait Connected extends QueryInterface {
  /** The priority of this connection, used to resolve conflicts. */
  val connectionPriority: ConnectionPriority

  /** Checks if this connection involves the specified instance.
    *
    * @param inst
    *   The chip instance to check.
    * @return
    *   True if the connection involves the specified instance.
    */
  def connectedTo(inst: ChipInstance): Boolean

  /** Checks if this connection is between two specified instances with
    * bidirectional flow.
    *
    * @param s
    *   The first chip instance.
    * @param e
    *   The second chip instance.
    * @return
    *   True if the connection is between the two instances.
    */
  def connectedBetween(s: ChipInstance, e: ChipInstance): Boolean =
    connectedBetween(s, e, BiDirectionConnection())

  /** Checks if this connection is between two specified instances with a
    * specific direction.
    *
    * @param s
    *   The first chip instance.
    * @param e
    *   The second chip instance.
    * @param d
    *   The connection direction to check.
    * @return
    *   True if the connection is between the two instances with the specified
    *   direction.
    */
  def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean

  /** Returns the first (source) instance location in this connection.
    *
    * @return
    *   Option containing the first instance location, or None if not
    *   applicable.
    */
  def first: Option[InstanceLoc]

  /** Returns the direction of the connection (input, output, bidirectional).
    *
    * @return
    *   The direction of this connection.
    */
  def direction: ConnectionDirection

  /** Returns the second (target) instance location in this connection.
    *
    * @return
    *   Option containing the second instance location, or None if not
    *   applicable.
    */
  def second: Option[InstanceLoc]

  /** Returns the fully qualified name of the first instance.
    *
    * @return
    *   The complete hierarchical name of the first instance.
    */
  def firstFullName: String

  /** Returns the fully qualified name of the second instance.
    *
    * @return
    *   The complete hierarchical name of the second instance.
    */
  def secondFullName: String

  /** Returns the last segment of the first instance's fully qualified name.
    *
    * @return
    *   The last part of the first instance's name (after the last dot).
    */
  def firstLastName: String = firstFullName.split('.').last

  /** Returns the last segment of the second instance's fully qualified name.
    *
    * @return
    *   The last part of the second instance's name (after the last dot).
    */
  def secondLastName: String = secondFullName.split('.').last

  /** Returns the first segment of the first instance's fully qualified name.
    *
    * @return
    *   The first part of the first instance's name (before the first dot).
    */
  def firstHeadName: String = firstFullName.split('.').head

  /** Returns the first segment of the second instance's fully qualified name.
    *
    * @return
    *   The first part of the second instance's name (before the first dot).
    */
  def secondHeadName: String = secondFullName.split('.').head

  /** Checks if this connection is from a pin to a chip.
    *
    * @return
    *   True if this connection goes from a pin to a chip.
    */
  def isPinToChip: Boolean

  /** Checks if this connection is between two chips.
    *
    * @return
    *   True if this connection goes between two chips.
    */
  def isChipToChip: Boolean

  /** Checks if this connection is from a chip to a pin.
    *
    * @return
    *   True if this connection goes from a chip to a pin.
    */
  def isChipToPin: Boolean

  /** Checks if this connection involves a clock.
    *
    * @return
    *   True if this connection involves a clock instance.
    */
  def isClock: Boolean
}
