package overlord.Connections

import overlord.Instances.{ChipInstance, InstanceTrait}

case class UnconnectedLogical(firstFullName: String,
                              direction: ConnectionDirection,
                              secondFullName: String) extends Unconnected {
	override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = {
		val mo = matchInstances(firstFullName, unexpanded)
		val so = matchInstances(secondFullName, unexpanded)

		val cbp = if (mo.length > 1 || so.length > 1)
			WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {mloc <- mo; sloc <- so} yield ConnectedLogical(cbp, mloc, direction, sloc)
	}

	override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

	override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] = Seq()

	override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
