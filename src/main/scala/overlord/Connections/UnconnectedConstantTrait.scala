package overlord.Connections

import ikuy_utils.Variant
import overlord.Instances.ChipInstance

case class UnConnectedConstant(firstFullName: String,
                               direction: ConnectionDirection,
                               secondFullName: String,
                               constant: Variant
                              ) extends UnConnected {

	override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = {
		val to  = matchInstances(secondFullName, unexpanded)
		val ccp = if (to.length > 1) WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {tloc <- to} yield ConnectedConstant(ccp, constant, direction, tloc)
	}

	override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

	override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
