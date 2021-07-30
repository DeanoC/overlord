package overlord.Connections

import overlord.Chip.Port
import overlord.{ChipDefinitionTrait, GatewareDefinitionTrait}
import overlord.Instances.{ChipInstance, ClockInstance, PinGroupInstance}
import toml.Value

sealed trait ConnectionPriority

case class GroupConnectionPriority() extends ConnectionPriority
case class WildCardConnectionPriority() extends ConnectionPriority
case class ExplicitConnectionPriority() extends ConnectionPriority

case class InstanceLoc(instance: ChipInstance,
                       port: Option[Port],
                       fullName: String) {
	def definition: ChipDefinitionTrait = instance.definition

	def isGateware: Boolean = instance.isInstanceOf[GatewareDefinitionTrait]
	def isPin: Boolean = instance.isInstanceOf[PinGroupInstance]
	def isClock: Boolean = instance.isInstanceOf[ClockInstance]
	def isChip:Boolean = !(isPin || isClock)

}

trait Connected extends Connection {
	val connectionPriority: ConnectionPriority

	def connectsToInstance(inst: ChipInstance): Boolean

	def first: Option[InstanceLoc]

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
