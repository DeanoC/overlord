package overlord.Connections

import overlord.Definitions.{
	ClockDefinitionType, DefinitionTrait,
	DefinitionType, PortDefinitionType
}
import overlord.Instances.Instance

case class ConnectedBetween(connectionType: ConnectionType,
                            connectionPriority: ConnectionPriority,
                            main: Instance,
                            secondary: Instance,
                            mainFullName: String,
                            secondaryFullName: String)
	extends Connected {
	override def connectsToInstance(inst: Instance): Boolean =
		(main == inst || secondary == inst)

	override def first: Option[Instance] = Some(main)

	override def second: Option[Instance] = Some(secondary)

	override def areConnectionCountsCompatible: Boolean = {
		val sharedOkay = main.attributes.contains("shared") ||
		                 secondary.attributes.contains("shared")

		assert((sharedOkay && firstCount == 1) ||
		       (sharedOkay && secondaryCount == 1) ||
		       (!sharedOkay))
		(main.count == secondary.count) || sharedOkay
	}

	override def firstCount: Int = main.count

	override def secondaryCount: Int = secondary.count

	def mainType: DefinitionType = main.definition.defType

	def secondaryType: DefinitionType = secondary.definition.defType

	def isPortToChip: Boolean = {
		main.definition.defType.isInstanceOf[PortDefinitionType] &&
		!secondary.definition.defType.isInstanceOf[PortDefinitionType]
	}

	def isPortToPort: Boolean = {
		main.definition.defType.isInstanceOf[PortDefinitionType] &&
		secondary.definition.defType.isInstanceOf[PortDefinitionType]
	}

	def isChipToChip: Boolean = {
		!main.definition.defType.isInstanceOf[PortDefinitionType] &&
		!secondary.definition.defType.isInstanceOf[PortDefinitionType]
	}

	def isChipToPort: Boolean = {
		!main.definition.defType.isInstanceOf[PortDefinitionType] &&
		secondary.definition.defType.isInstanceOf[PortDefinitionType]
	}

	def isClock: Boolean = {
		main.definition.defType.isInstanceOf[ClockDefinitionType] ||
		secondary.definition.defType.isInstanceOf[ClockDefinitionType]
	}

	override def firstFullName: String = mainFullName

	override def secondFullName: String = secondaryFullName
}
