package com.deanoc.overlord.Connections

import com.deanoc.overlord._
import com.deanoc.overlord.Instances.ChipInstance
import com.deanoc.overlord.Interfaces.SupplierBusLike

case class ConnectedBus(
    connectionPriority: ConnectionPriority,
    main: InstanceLoc,
    direction: ConnectionDirection,
    secondary: InstanceLoc,
    bus: SupplierBusLike,
    other: ChipInstance
) extends ConnectedBetween {
  override def first: Option[InstanceLoc] = Some(main)

  override def second: Option[InstanceLoc] = Some(secondary)

}
