package overlord.Connections
import overlord._

import overlord.Hardware.{InOutWireDirection, Port}
import overlord.{ConnectionDirection, FirstToSecondConnection}

import overlord.Instances.ChipInstance
import overlord.PortsLike

case class ConnectedPortGroup(connectionPriority: ConnectionPriority,
                              main: InstanceLoc,
                              direction: ConnectionDirection,
                              secondary: InstanceLoc) extends Connected {
	override def connectedTo(inst: ChipInstance): Boolean = (main.instance.name == inst.name) || (secondary.instance.name == inst.name)

	override def first: Option[InstanceLoc] = Some(main)

	override def second: Option[InstanceLoc] = Some(secondary)

	override def firstFullName: String = main.instance.name

	override def secondFullName: String = secondary.instance.name

	override def isPinToChip: Boolean = main.isPin && secondary.isChip

	override def isChipToChip: Boolean = main.isChip && secondary.isChip

	override def isChipToPin: Boolean = main.isChip && secondary.isPin

	override def isClock: Boolean = false

	override def connectedBetween(s: ChipInstance, e: ChipInstance, d: ConnectionDirection): Boolean = {
		d match {
			case FirstToSecondConnection() => (main.instance == s && secondary.instance == e)
			case SecondToFirstConnection() => (main.instance == e && secondary.instance == s)
			case BiDirectionConnection()   => ((main.instance == s && secondary.instance == e) || (main.instance == e && secondary.instance == s))
		}
	}
}

object ConnectedPortGroup {
	def apply(fi: PortsLike,
	          fp: Port,
	          fn: String,
	          si: PortsLike,
	          sp: Port,
	          direction: ConnectionDirection): ConnectedPortGroup = {
		var firstDirection  = fp.direction
		var secondDirection = sp.direction

		if (fp.direction != InOutWireDirection()) {
			if (sp.direction == InOutWireDirection()) secondDirection = fp.direction
		} else if (sp.direction != InOutWireDirection()) firstDirection = sp.direction

		val fport = fp.copy(direction = firstDirection)
		val sport = sp.copy(direction = secondDirection)

		val fmloc = InstanceLoc(fi.getOwner, Some(fport), s"$fn.${fp.name}")
		val fsloc = InstanceLoc(si.getOwner, Some(sport), s"$fn.${sp.name}")

		ConnectedPortGroup(GroupConnectionPriority(), fmloc, direction, fsloc)
	}
}
