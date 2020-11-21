package overlord.Connections

import overlord.Instances.Instance

sealed trait ConnectionPriority

case class GroupConnectionPriority() extends ConnectionPriority
case class WildCardConnectionPriority() extends ConnectionPriority
case class ExplicitConnectionPriority() extends ConnectionPriority

trait Connected extends Connection {
	val connectionPriority: ConnectionPriority

	def connectsToInstance(inst: Instance): Boolean

	def areConnectionCountsCompatible: Boolean

	def firstCount: Int

	def secondaryCount: Int

	def first: Option[Instance]

	def second: Option[Instance]

	def firstFullName: String

	def secondFullName: String

	def firstLastName: String = firstFullName.split('.').last

	def secondLastName: String = secondFullName.split('.').last

	def firstHeadName: String = firstFullName.split('.').head

	def secondHeadName: String = secondFullName.split('.').head

	def isPortToChip: Boolean

	def isChipToChip: Boolean

	def isChipToPort: Boolean

	def isClock: Boolean
}
