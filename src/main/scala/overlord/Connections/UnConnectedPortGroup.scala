package overlord.Connections

import overlord.Instances.ChipInstance
import overlord.Interfaces.PortsLike

case class UnConnectedPortGroup(firstFullName: String,
                                direction: ConnectionDirection,
                                secondFullName: String,
                                first_prefix: String,
                                second_prefix: String,
                                excludes: Seq[String]) extends UnConnected {
	override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = for {
		mloc <- matchInstances(firstFullName, unexpanded)
		sloc <- matchInstances(secondFullName, unexpanded)
		if mloc.instance.hasInterface[PortsLike]
		if sloc.instance.hasInterface[PortsLike]
		mi = mloc.instance.getInterfaceUnwrapped[PortsLike]
		si = sloc.instance.getInterfaceUnwrapped[PortsLike]
		fp <- mi.getPortsStartingWith(first_prefix)
		sp <- si.getPortsStartingWith(second_prefix)
		if fp.name.stripPrefix(first_prefix) == sp.name.stripPrefix(second_prefix)
		if !(excludes.contains(fp.name) || excludes.contains(sp.name))
	} yield ConnectedPortGroup(mi, fp, mloc.fullName, si, sp, direction)

	override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

	override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}

