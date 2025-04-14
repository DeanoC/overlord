package com.deanoc.overlord.connections

import com.deanoc.overlord._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.instances.ChipInstance
import com.deanoc.overlord.interfaces.SupplierBusLike
import com.deanoc.overlord.connections.ConnectedExtensions._

/** Represents a bus connection between two components in the system.
  *
  * A bus connection is a specific type of connection that involves a data bus
  * between components. Bus connections typically have a provider (source) and
  * consumer (target) and can carry complex data structures between components.
  *
  * In configuration files (like `fabric.yaml`), bus connections are defined
  * with:
  *   - `type`: bus
  *   - `bus_name`: The name of the bus on the source side
  *   - `connection`: The source and target components separated by `->`
  *   - `consumer_bus_name`: (Optional) The name of the bus on the target side
  *     if different
  *
  * Example from `fabric.yaml`:
  * ```yaml
  * - bus_name: fpd_ocm
  *   connection: FPDMainSwitch -> OCMSwitch
  *   type: bus
  * ```
  *
  * @param connectionPriority
  *   The priority of this connection.
  * @param main
  *   The main/source instance location.
  * @param direction
  *   The direction of the connection.
  * @param secondary
  *   The secondary/target instance location.
  * @param bus
  *   The bus interface used for this connection.
  * @param other
  *   The other chip instance in this connection.
  */
case class ConnectedBus(
    connectionPriority: ConnectionPriority,
    main: InstanceLoc,
    direction: ConnectionDirection,
    secondary: InstanceLoc,
    bus: SupplierBusLike,
    other: ChipInstance
) extends ConnectedBetween {

  /** The name of this connection */
  override def connectionName: ConnectionTypes.ConnectionName =
    ConnectionTypes.ConnectionName(s"${firstFullName}_to_${secondFullName}_bus")

  /** Returns the first (source) instance location in this connection. */
  override def first: Option[InstanceLoc] = Some(main)

  /** Returns the second (target) instance location in this connection. */
  override def second: Option[InstanceLoc] = Some(secondary)

  // Implement abstract methods from ConnectedBetween
  override def connectedTo(inst: ChipInstance): Boolean =
    (first.nonEmpty && first.get.instance.name == inst.name) ||
      (second.nonEmpty && second.get.instance.name == inst.name)

  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean =
    if (first.isEmpty || second.isEmpty) false
    else
      d match
        case ConnectionDirection.FirstToSecond =>
          first.get.instance == s && second.get.instance == e
        case ConnectionDirection.SecondToFirst =>
          first.get.instance == e && second.get.instance == s
        case ConnectionDirection.BiDirectional =>
          (first.get.instance == s && second.get.instance == e) ||
          (first.get.instance == e && second.get.instance == s)

  override def isPinToChip: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isPin && second.get.isChip

  override def isChipToChip: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isChip && second.get.isChip

  override def isChipToPin: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isChip && second.get.isPin

  override def isClock: Boolean =
    (first.nonEmpty && first.get.isClock) || (second.nonEmpty && second.get.isClock)

  override def firstFullName: String =
    if (first.nonEmpty) first.get.fullName else "NOT_CONNECTED"

  override def secondFullName: String =
    if (second.nonEmpty) second.get.fullName else "NOT_CONNECTED"
}
