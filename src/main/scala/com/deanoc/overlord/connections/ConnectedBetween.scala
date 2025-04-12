package com.deanoc.overlord.Connections

import com.deanoc.overlord._
import com.deanoc.overlord.DefinitionType
import com.deanoc.overlord.Instances.ChipInstance

/**
  * Trait representing a connection between two components with a defined priority and direction.
  */
trait ConnectedBetween extends Connected {
  /** The priority of the connection. */
  val connectionPriority: ConnectionPriority

  /** The direction of the connection. */
  val direction: ConnectionDirection

  /**
    * Retrieves the definition type of the main (first) instance, if available.
    *
    * @return An optional `DefinitionType` of the main instance.
    */
  def mainType: Option[DefinitionType] =
    if (first.nonEmpty) Some(first.get.definition.defType) else None

  /**
    * Retrieves the definition type of the secondary (second) instance, if available.
    *
    * @return An optional `DefinitionType` of the secondary instance.
    */
  def secondaryType: Option[DefinitionType] =
    if (second.nonEmpty) Some(second.get.definition.defType) else None

  /**
    * Checks if the connection involves the specified chip instance.
    *
    * @param inst The chip instance to check.
    * @return True if the connection involves the specified instance, false otherwise.
    */
  override def connectedTo(inst: ChipInstance): Boolean =
    (first.nonEmpty && first.get.instance.name == inst.name) || (second.nonEmpty && second.get.instance.name == inst.name)

  /**
    * Determines if the connection exists between two chip instances in the specified direction.
    *
    * @param s The source chip instance.
    * @param e The target chip instance.
    * @param d The direction of the connection.
    * @return True if the connection exists, false otherwise.
    */
  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean = {
    if (first.isEmpty || second.isEmpty) false
    else {
      d match {
        case FirstToSecondConnection() => (
          first.get.instance == s && second.get.instance == e
        )
        case SecondToFirstConnection() => (
          first.get.instance == e && second.get.instance == s
        )
        case BiDirectionConnection() => (
          (first.get.instance == s && second.get.instance == e) || (first.get.instance == e && second.get.instance == s)
        )
      }
    }
  }

  /**
    * Checks if the connection is between a pin and a chip.
    *
    * @return True if the connection is between a pin and a chip, false otherwise.
    */
  override def isPinToChip: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isPin && second.get.isChip

  /**
    * Checks if the connection is between two chips.
    *
    * @return True if the connection is between two chips, false otherwise.
    */
  override def isChipToChip: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isChip && second.get.isChip

  /**
    * Checks if the connection is between a chip and a pin.
    *
    * @return True if the connection is between a chip and a pin, false otherwise.
    */
  override def isChipToPin: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isChip && second.get.isPin

  /**
    * Checks if the connection involves a clock instance.
    *
    * @return True if the connection involves a clock, false otherwise.
    */
  override def isClock: Boolean =
    (first.nonEmpty && first.get.isClock) || (second.nonEmpty && second.get.isClock)

  /**
    * Retrieves the full name of the first instance in the connection.
    *
    * @return The full name of the first instance, or "NOT_CONNECTED" if unavailable.
    */
  override def firstFullName: String =
    if (first.nonEmpty) first.get.fullName else "NOT_CONNECTED"

  /**
    * Retrieves the full name of the second instance in the connection.
    *
    * @return The full name of the second instance, or "NOT_CONNECTED" if unavailable.
    */
  override def secondFullName: String =
    if (second.nonEmpty) second.get.fullName else "NOT_CONNECTED"

}
