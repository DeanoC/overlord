package overlord.Connections

import overlord.Instances.ChipInstance

case class UnConnectedLogical(firstFullName: String,
                              direction: ConnectionDirection,
                              secondFullName: String) extends UnConnected {
	override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = {
		val mo = matchInstances(firstFullName, unexpanded)
		val so = matchInstances(secondFullName, unexpanded)

		val cbp = if (mo.length > 1 || so.length > 1)
			WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {mloc <- mo; sloc <- so} yield ConnectedLogical(cbp, mloc, direction, sloc)
	}

	override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

	override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
