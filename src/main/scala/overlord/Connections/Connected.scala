package overlord.Connections

import overlord.Definitions.DefinitionTrait
import overlord.Gateware.Port
import overlord.Instances.{ClockInstance, Instance, PinGroupInstance}
import toml.Value

sealed trait ConnectionPriority

case class GroupConnectionPriority() extends ConnectionPriority
case class WildCardConnectionPriority() extends ConnectionPriority
case class ExplicitConnectionPriority() extends ConnectionPriority

case class InstanceLoc(instance: Instance,
                       port: Option[Port],
                       fullName: String) {
	def definition: DefinitionTrait = instance.definition

	def attributes: Map[String, Value] = instance.attributes

	def isPin: Boolean = instance.isInstanceOf[PinGroupInstance]
	def isClock: Boolean = instance.isInstanceOf[ClockInstance]
	def isChip:Boolean = !(isPin || isClock)

}

trait Connected extends Connection {
	val connectionPriority: ConnectionPriority

	def connectsToInstance(inst: Instance): Boolean

	def areConnectionCountsCompatible: Boolean

	def firstCount: Int

	def secondaryCount: Int

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
