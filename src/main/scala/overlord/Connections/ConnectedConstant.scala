package overlord.Connections

import overlord.Instances.Instance

import ikuy_utils.Variant

case class ConnectedConstant(connectionType: ConnectionType,
                             connectionPriority: ConnectionPriority,
                             constant: Variant,
                             direction: ConnectionDirection,
                             to: InstanceLoc)
	extends Connected {
	override def connectsToInstance(inst: Instance): Boolean =
		to.instance == inst

	override def firstCount: Int = to.instance.replicationCount

	override def secondaryCount: Int = to.instance.replicationCount

	override def first: Option[InstanceLoc] = None

	override def second: Option[InstanceLoc] = Some(to)

	override def areConnectionCountsCompatible: Boolean = true

	def asParameter: Map[String, Variant] = Map(to.fullName -> constant)

	override def firstFullName: String = "CONSTANT"

	override def secondFullName: String = to.fullName

	override def isChipToChip: Boolean = false

	override def isChipToPin: Boolean = false

	override def isPinToChip: Boolean = false

	override def isClock: Boolean = false

}
