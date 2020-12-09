package overlord.Connections

import overlord.Definitions.{ClockDefinitionType, DefinitionTrait, DefinitionType}
import overlord.Gateware.{Port, WireDirection}
import overlord.Instances.{ClockInstance, Instance, PinGroupInstance}
import toml.Value

case class ConnectedBetween(connectionType: ConnectionType,
                            connectionPriority: ConnectionPriority,
                            main: InstanceLoc,
                            direction: ConnectionDirection,
                            secondary: InstanceLoc)
	extends Connected {

	override def connectsToInstance(inst: Instance): Boolean =
		(main.instance == inst || secondary.instance == inst)

	override def first: Option[InstanceLoc] = Some(main)

	override def second: Option[InstanceLoc] = Some(secondary)

	override def areConnectionCountsCompatible: Boolean = {
		val sharedOkay = main.attributes.contains("shared") ||
		                 secondary.attributes.contains("shared")

		assert((sharedOkay && firstCount == 1) ||
		       (sharedOkay && secondaryCount == 1) ||
		       (!sharedOkay))

		(firstCount == secondaryCount) || sharedOkay
	}

	override def firstCount: Int = main.instance.count

	override def secondaryCount: Int = secondary.instance.count

	def mainType: DefinitionType = main.definition.defType

	def secondaryType: DefinitionType = secondary.definition.defType

	def isPinToChip: Boolean = main.isPin && secondary.isChip

	def isChipToChip: Boolean = main.isChip && secondary.isChip

	def isChipToPin: Boolean = main.isChip && secondary.isPin

	def isClock: Boolean = main.isClock || secondary.isClock

	override def firstFullName: String = main.fullName

	override def secondFullName: String = secondary.fullName
}
