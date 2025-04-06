package overlord.Connections
import overlord._


case class ConnectedLogical(connectionPriority: ConnectionPriority,
                            main: InstanceLoc,
                            direction: ConnectionDirection,
                            secondary: InstanceLoc) extends ConnectedBetween {
	override def first: Option[InstanceLoc] = Some(main)

	override def second: Option[InstanceLoc] = Some(secondary)
}
