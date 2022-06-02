package overlord.Connections

import ikuy_utils.Variant
import overlord.Instances.ChipInstance

case class ConnectedConstant(connectionPriority: ConnectionPriority,
                             constant: Variant,
                             direction: ConnectionDirection,
                             secondary: InstanceLoc) extends ConnectedBetween {
	override def connectedTo(inst: ChipInstance): Boolean = secondary.instance == inst

	override def connectedBetween(s: ChipInstance, e: ChipInstance, d: ConnectionDirection): Boolean = false

	override def first: Option[InstanceLoc] = None

	override def second: Option[InstanceLoc] = Some(secondary)

	def asParameter: Map[String, Variant] = Map(secondary.fullName -> constant)

	override def firstFullName: String = "CONSTANT"

	override def secondFullName: String = secondary.fullName

	override def isChipToChip: Boolean = false

	override def isChipToPin: Boolean = false

	override def isPinToChip: Boolean = false

	override def isClock: Boolean = false

}
