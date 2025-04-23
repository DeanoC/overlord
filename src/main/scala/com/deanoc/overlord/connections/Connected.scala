package com.deanoc.overlord.connections

import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.definitions.{ 
  DefinitionTrait,
  GatewareDefinition,
  HardwareDefinition,
  SoftwareDefinitionTrait
}
import com.deanoc.overlord.QueryInterface

import com.deanoc.overlord.instances.{
  HardwareInstance,
  ClockInstance,
  InstanceTrait,
  PinGroupInstance
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

  /** The name of this connection */
  def connectionName: ConnectionTypes.ConnectionName

  /** Checks if this connection involves the specified instance.
    *
    * @param inst
    *   The chip instance to check.
    * @return
    *   True if the connection involves the specified instance.
    */
  def connectedTo(inst: HardwareInstance): Boolean

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
  def connectedBetween(s: HardwareInstance, e: HardwareInstance): Boolean =
    connectedBetween(s, e, ConnectionDirection.BiDirectional)

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
      s: HardwareInstance,
      e: HardwareInstance,
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

  // Name operations moved to extension methods in ConnectedExtensions

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
