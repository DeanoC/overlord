package com.deanoc.overlord.connections

import com.deanoc.overlord._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.instances.HardwareInstance

/** Represents a logical connection between two components in the system.
  *
  * A logical connection establishes a relationship between components without
  * the specific data transfer characteristics of a bus. These connections
  * define architectural relationships and control flows between components.
  *
  * In configuration files (like `fabric.yaml`), logical connections are defined
  * with:
  *   - `type`: logical
  *   - `connection`: The source and target components separated by `->`
  *
  * Example from `fabric.yaml`:
  * ```yaml
  * - connection: FPDMainSwitch -> LPDInboundSwitch
  *   type: logical
  * ```
  *
  * @param connectionPriority
  *   The priority of this connection.
  * @param main
  *   The main/source instance location.
  * @param direction
  *   The direction of the connection flow.
  * @param secondary
  *   The secondary/target instance location.
  */
case class ConnectedLogical(
    connectionPriority: ConnectionPriority,
    main: InstanceLoc,
    direction: ConnectionDirection,
    secondary: InstanceLoc
) extends ConnectedBetween {

  /** The name of this connection */
  override def connectionName: ConnectionTypes.ConnectionName =
    ConnectionTypes.ConnectionName(
      s"${firstFullName}_to_${secondFullName}_logical"
    )

  /** Returns the first (source) instance location in this connection. */
  override def first: Option[InstanceLoc] = Some(main)

  /** Returns the second (target) instance location in this connection. */
  override def second: Option[InstanceLoc] = Some(secondary)

  // Implement abstract methods from ConnectedBetween
  override def connectedTo(inst: HardwareInstance): Boolean =
    (first.nonEmpty && first.get.instance.name == inst.name) ||
      (second.nonEmpty && second.get.instance.name == inst.name)

  override def connectedBetween(
      s: HardwareInstance,
      e: HardwareInstance,
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
