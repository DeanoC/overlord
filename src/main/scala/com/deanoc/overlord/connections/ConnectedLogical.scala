package com.deanoc.overlord.connections

import com.deanoc.overlord._

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

  /** Returns the first (source) instance location in this connection. */
  override def first: Option[InstanceLoc] = Some(main)

  /** Returns the second (target) instance location in this connection. */
  override def second: Option[InstanceLoc] = Some(secondary)
}
