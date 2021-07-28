package overlord.Connections

import overlord.Definitions.DefinitionType
import overlord.Instances.Instance

case class ConnectedBetween(connectionType: ConnectionType,
                            connectionPriority: ConnectionPriority,
                            main: InstanceLoc,
                            direction: ConnectionDirection,
                            secondary: InstanceLoc)
	extends Connected {

	def mainType: DefinitionType = main.definition.defType

	def secondaryType: DefinitionType = secondary.definition.defType

	override def connectsToInstance(inst: Instance): Boolean =
		(main.instance.ident == inst.ident ||
		 secondary.instance.ident == inst.ident)

	override def first: Option[InstanceLoc] = Some(main)

	override def second: Option[InstanceLoc] = Some(secondary)

	override def isPinToChip: Boolean = main.isPin && secondary.isChip

	override def isChipToChip: Boolean = main.isChip && secondary.isChip

	override def isChipToPin: Boolean = main.isChip && secondary.isPin

	override def isClock: Boolean = main.isClock || secondary.isClock

	override def firstFullName: String = main.fullName

	override def secondFullName: String = secondary.fullName
}
