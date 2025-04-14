package com.deanoc.overlord.connections

import com.deanoc.overlord._
import com.deanoc.overlord.DefinitionType
import com.deanoc.overlord.instances.ChipInstance

/** Trait representing a connection between two components with a defined
  * priority and direction.
  */
trait ConnectedBetween extends Connected {

  /** The priority of the connection. */
  val connectionPriority: ConnectionPriority

  /** The direction of the connection. */
  val direction: ConnectionDirection

  /** Retrieves the definition type of the main (first) instance, if available.
    *
    * @return
    *   An optional `DefinitionType` of the main instance.
    */
  // Definition type methods moved to extension methods in ConnectedExtensions

  /** Checks if the connection involves the specified chip instance.
    *
    * @param inst
    *   The chip instance to check.
    * @return
    *   True if the connection involves the specified instance, false otherwise.
    */
  // Connection check moved to extension methods in ConnectedExtensions
  override def connectedTo(inst: ChipInstance): Boolean

  /** Determines if the connection exists between two chip instances in the
    * specified direction.
    *
    * @param s
    *   The source chip instance.
    * @param e
    *   The target chip instance.
    * @param d
    *   The direction of the connection.
    * @return
    *   True if the connection exists, false otherwise.
    */
  // Connection between check moved to extension methods in ConnectedExtensions
  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean

  /** Checks if the connection is between a pin and a chip.
    *
    * @return
    *   True if the connection is between a pin and a chip, false otherwise.
    */
  // Connection type check moved to extension methods in ConnectedExtensions
  override def isPinToChip: Boolean

  /** Checks if the connection is between two chips.
    *
    * @return
    *   True if the connection is between two chips, false otherwise.
    */
  // Connection type check moved to extension methods in ConnectedExtensions
  override def isChipToChip: Boolean

  /** Checks if the connection is between a chip and a pin.
    *
    * @return
    *   True if the connection is between a chip and a pin, false otherwise.
    */
  // Connection type check moved to extension methods in ConnectedExtensions
  override def isChipToPin: Boolean

  /** Checks if the connection involves a clock instance.
    *
    * @return
    *   True if the connection involves a clock, false otherwise.
    */
  // Clock connection check moved to extension methods in ConnectedExtensions
  override def isClock: Boolean

  /** Retrieves the full name of the first instance in the connection.
    *
    * @return
    *   The full name of the first instance, or "NOT_CONNECTED" if unavailable.
    */
  // Name retrieval moved to extension methods in ConnectedExtensions
  override def firstFullName: String

  /** Retrieves the full name of the second instance in the connection.
    *
    * @return
    *   The full name of the second instance, or "NOT_CONNECTED" if unavailable.
    */
  // Name retrieval moved to extension methods in ConnectedExtensions
  override def secondFullName: String

}
