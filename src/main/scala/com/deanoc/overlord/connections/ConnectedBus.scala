package com.deanoc.overlord.Connections

import com.deanoc.overlord._
import com.deanoc.overlord.Instances.ChipInstance
import com.deanoc.overlord.Interfaces.SupplierBusLike

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

  /** Returns the first (source) instance location in this connection. */
  override def first: Option[InstanceLoc] = Some(main)

  /** Returns the second (target) instance location in this connection. */
  override def second: Option[InstanceLoc] = Some(secondary)
}
