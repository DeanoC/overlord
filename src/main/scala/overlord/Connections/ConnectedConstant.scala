package overlord.Connections

import overlord.Definitions.DefinitionTrait
import overlord.Instances.Instance

case class ConnectedConstant(connectionType: ConnectionType,
                             connectionPriority: ConnectionPriority,
                             constant: String,
                             to: Instance,
                             toFullName: String)
	extends Connected {
	override def connectsToInstance(inst: Instance): Boolean = (to == inst)

	override def firstCount: Int = to.count

	override def secondaryCount: Int = to.count

	override def first: Option[Instance] = None

	override def second: Option[Instance] = Some(to)

	override def areConnectionCountsCompatible: Boolean = true

	def asParameter: Map[String, String] =
		Map[String, String](toFullName.split('.').last -> constant)

	override def firstFullName: String = constant

	override def secondFullName: String = toFullName

	override def isChipToChip: Boolean = false

	override def isChipToPort: Boolean = false

	override def isClock: Boolean = false

	override def isPortToChip: Boolean = false
}
