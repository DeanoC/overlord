package overlord

trait Connection {
	val definition: Definition

	def isUnconnected: Boolean = this.isInstanceOf[Unconnected]

	def asUnconnected: Unconnected = this.asInstanceOf[Unconnected]

	def isConnected: Boolean = this.isInstanceOf[Connected]

	def asConnected: Connected = this.asInstanceOf[Connected]
}

trait Unconnected extends Connection {
	def first: String
	def second: String
}

trait Connected extends Connection {
	def connectsToInstance(inst: Instance[_]): Boolean

	def areConnectionCountsCompatible: Boolean

	def firstCount: Int

	def secondaryCount: Int

	def first: Instance[_]

	def second: Instance[_]

	def unconnected: Unconnected

}

case class UnconnectedBetween(definition: Definition,
                              main: String,
                              secondary: String,
                             ) extends Unconnected {
	def first: String = main
	def second: String = secondary
}

case class UnconnectedConstant(definition: Definition,
                               constant: String,
                               to: String,
                              ) extends Unconnected{
	def first: String = constant
	def second: String = to
}


case class ConnectedBetween[
	A <: Instance[_], B <: Instance[_]](definition: Definition,
                                      main: A,
                                      secondary: B,
                                      unconnected: Unconnected)
	extends Connected {
	override def connectsToInstance(inst: Instance[_]): Boolean =
		(main == inst || secondary == inst)

	override def firstCount: Int = main.count

	override def secondaryCount: Int = secondary.count

	override def first: Instance[_] = main

	override def second: Instance[_] = secondary

	override def areConnectionCountsCompatible: Boolean = {
		val sharedOkay = main.attributes.contains("shared") ||
		                 secondary.attributes.contains("shared")

		assert((sharedOkay && firstCount == 1) ||
		       (sharedOkay && secondaryCount == 1) ||
		       (!sharedOkay))
		(main.count == secondary.count) || sharedOkay
	}
}

case class ConnectedConstant[B <: Instance[_]](definition: Definition,
                                               constant: String,
                                               to: B,
                                               unconnected: Unconnected)
	extends Connected {
	override def connectsToInstance(inst: Instance[_]): Boolean = (to == inst)

	override def firstCount: Int = to.count

	override def secondaryCount: Int = to.count

	override def first: Instance[_] = to

	override def second: Instance[_] = to

	override def areConnectionCountsCompatible: Boolean = true
}

