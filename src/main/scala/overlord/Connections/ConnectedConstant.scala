package overlord.Connections

import overlord.Gateware.{Parameter, WireDirection}
import overlord.Instances.Instance

import scala.collection.mutable

case class ConnectedConstant(connectionType: ConnectionType,
                             connectionPriority: ConnectionPriority,
                             constant: BigInt,
                             direction: ConnectionDirection,
                             to: InstanceLoc)
	extends Connected {
	override def connectsToInstance(inst: Instance): Boolean =
		to.instance == inst

	override def firstCount: Int = to.instance.count

	override def secondaryCount: Int = to.instance.count

	override def first: Option[InstanceLoc] = None

	override def second: Option[InstanceLoc] = Some(to)

	override def areConnectionCountsCompatible: Boolean = true

	def asParameter: mutable.HashMap[String, Parameter] = {
		val name = to.fullName
		mutable.HashMap[String, Parameter]((name, Parameter(name, constant)))
	}

	override def firstFullName: String = "CONSTANT"

	override def secondFullName: String = to.fullName

	override def isChipToChip: Boolean = false

	override def isChipToPin: Boolean = false

	override def isPinToChip: Boolean = false

	override def isClock: Boolean = false

}
