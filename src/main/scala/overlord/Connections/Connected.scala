package overlord.Connections

import overlord.Chip.Port
import overlord.Instances.{ChipInstance, ClockInstance, InstanceTrait, PinGroupInstance}
import overlord.Interfaces.QueryInterface
import overlord.{DefinitionTrait, GatewareDefinitionTrait, HardwareDefinitionTrait, SoftwareDefinitionTrait}

sealed trait ConnectionPriority

case class GroupConnectionPriority() extends ConnectionPriority

case class WildCardConnectionPriority() extends ConnectionPriority

case class ExplicitConnectionPriority() extends ConnectionPriority

case class InstanceLoc(instance: InstanceTrait,
                       port: Option[Port],
                       fullName: String) {
	def definition: DefinitionTrait = instance.definition

	val isHardware: Boolean = definition.isInstanceOf[HardwareDefinitionTrait]

	def isGateware: Boolean = definition.isInstanceOf[GatewareDefinitionTrait]

	def isSoftware: Boolean = definition.isInstanceOf[SoftwareDefinitionTrait]

	def isPin: Boolean = instance.isInstanceOf[PinGroupInstance]

	def isClock: Boolean = instance.isInstanceOf[ClockInstance]

	def isChip: Boolean = !(isPin || isClock)

}

trait Connected extends QueryInterface {
	val connectionPriority: ConnectionPriority

	def connectedTo(inst: ChipInstance): Boolean

	def connectedBetween(s: ChipInstance, e: ChipInstance): Boolean = connectedBetween(s, e, BiDirectionConnection())

	def connectedBetween(s: ChipInstance, e: ChipInstance, d: ConnectionDirection): Boolean

	def first: Option[InstanceLoc]

	def direction: ConnectionDirection

	def second: Option[InstanceLoc]

	def firstFullName: String

	def secondFullName: String

	def firstLastName: String = firstFullName.split('.').last

	def secondLastName: String = secondFullName.split('.').last

	def firstHeadName: String = firstFullName.split('.').head

	def secondHeadName: String = secondFullName.split('.').head

	def isPinToChip: Boolean

	def isChipToChip: Boolean

	def isChipToPin: Boolean

	def isClock: Boolean
}
