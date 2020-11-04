package overlord


trait Connection {
	val ident     : String
	val definition: Definition

	def connectsToInstance(inst: Instance[_]): Boolean
}

case class Unconnected(ident: String,
                       definition: Definition,
                       main: String,
                       secondary: String,
                      ) extends Connection {
	override def connectsToInstance(inst: Instance[_]): Boolean = false
}

case class Connected[A <: Instance[_], B <: Instance[_]](ident: String,
                                                         definition: Definition,
                                                         main: A,
                                                         secondary: B,
                                                        ) extends Connection {
	override def connectsToInstance(inst: Instance[_]): Boolean =
		(main == inst || secondary == inst)

	def mainCount: Int = main.count

	def secondaryCount: Int = secondary.count

	def areConnectionCountsCompatible: Boolean = {
		val sharedOkay = main.attributes.contains("shared") ||
		                 secondary.attributes.contains("shared")

		assert((sharedOkay && mainCount == 1) ||
		       (sharedOkay && secondaryCount == 1) ||
		       (!sharedOkay))
		(main.count == secondary.count) || sharedOkay
	}
}

