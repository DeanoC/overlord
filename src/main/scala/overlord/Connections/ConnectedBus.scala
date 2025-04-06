package overlord.Connections

import overlord._
import overlord.Instances.ChipInstance
import overlord.SupplierBusLike

case class ConnectedBus(connectionPriority: ConnectionPriority,
                        main: InstanceLoc,
                        direction: ConnectionDirection,
                        secondary: InstanceLoc,
                        bus: SupplierBusLike,
                        other: ChipInstance,
                       ) extends ConnectedBetween {
	override def first: Option[InstanceLoc] = Some(main)

	override def second: Option[InstanceLoc] = Some(secondary)

}
