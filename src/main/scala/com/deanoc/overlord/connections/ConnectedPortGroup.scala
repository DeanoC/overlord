package com.deanoc.overlord.connections
import com.deanoc.overlord._

import com.deanoc.overlord.hardware.{InOutWireDirection, Port}
import com.deanoc.overlord.ConnectionDirection

import com.deanoc.overlord.instances.ChipInstance
import com.deanoc.overlord.interfaces.PortsLike

/** Represents a group of connected ports between two components.
  *
  * This case class defines the connection between two instances, including
  * their ports and connection direction.
  *
  * @param connectionPriority
  *   The priority of this connection.
  * @param main
  *   The main/source instance location.
  * @param direction
  *   The direction of the connection.
  * @param secondary
  *   The secondary/target instance location.
  */
case class ConnectedPortGroup(
    connectionPriority: ConnectionPriority,
    main: InstanceLoc,
    direction: ConnectionDirection,
    secondary: InstanceLoc
) extends Connected {

  /** Checks if the connection involves the specified chip instance. */
  override def connectedTo(inst: ChipInstance): Boolean =
    (main.instance.name == inst.name) || (secondary.instance.name == inst.name)

  /** Returns the first (source) instance location in this connection. */
  override def first: Option[InstanceLoc] = Some(main)

  /** Returns the second (target) instance location in this connection. */
  override def second: Option[InstanceLoc] = Some(secondary)

  /** Retrieves the full name of the first instance. */
  override def firstFullName: String = main.instance.name

  /** Retrieves the full name of the second instance. */
  override def secondFullName: String = secondary.instance.name

  /** Checks if the connection is between a pin and a chip. */
  override def isPinToChip: Boolean = main.isPin && secondary.isChip

  /** Checks if the connection is between two chips. */
  override def isChipToChip: Boolean = main.isChip && secondary.isChip

  /** Checks if the connection is between a chip and a pin. */
  override def isChipToPin: Boolean = main.isChip && secondary.isPin

  /** Checks if the connection involves a clock. */
  override def isClock: Boolean = false

  /** Determines if the connection exists between two chip instances in the
    * specified direction.
    */
  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean = {
    d match {
      case ConnectionDirection.FirstToSecond => (
        main.instance == s && secondary.instance == e
      )
      case ConnectionDirection.SecondToFirst => (
        main.instance == e && secondary.instance == s
      )
      case ConnectionDirection.BiDirectional => (
        (main.instance == s && secondary.instance == e) || (main.instance == e && secondary.instance == s)
      )
    }
  }
}

/** Factory object for creating instances of `ConnectedPortGroup`. */
object ConnectedPortGroup {

  /** Creates a `ConnectedPortGroup` instance.
    *
    * @param fi
    *   The first port's owner.
    * @param fp
    *   The first port.
    * @param fn
    *   The name of the first port.
    * @param si
    *   The second port's owner.
    * @param sp
    *   The second port.
    * @param direction
    *   The direction of the connection.
    * @return
    *   A new `ConnectedPortGroup` instance.
    */
  def apply(
      fi: PortsLike,
      fp: Port,
      fn: String,
      si: PortsLike,
      sp: Port,
      direction: ConnectionDirection
  ): ConnectedPortGroup = {
    // Handle wire directions - wires need to be properly directed
    var firstDirection = fp.direction
    var secondDirection = sp.direction

    if (fp.direction != InOutWireDirection()) {
      if (sp.direction == InOutWireDirection()) secondDirection = fp.direction
    } else if (sp.direction != InOutWireDirection())
      firstDirection = sp.direction

    val fport = fp.copy(direction = firstDirection)
    val sport = sp.copy(direction = secondDirection)

    // Create instance locations with updated port directions
    val fmloc = InstanceLoc(fi.getOwner, Some(fport), s"$fn.${fp.name}")
    val fsloc = InstanceLoc(si.getOwner, Some(sport), s"$fn.${sp.name}")

    // Create the connected port group with Group priority
    ConnectedPortGroup(ConnectionPriority.Group, fmloc, direction, fsloc)
  }
}
