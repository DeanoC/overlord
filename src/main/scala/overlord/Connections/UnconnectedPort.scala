package overlord.Connections

import overlord.Instances.{ChipInstance, InstanceTrait}
import overlord.Interfaces.PortsLike

case class UnconnectedPort(firstFullName: String,
                           direction: ConnectionDirection,
                           secondFullName: String) extends Unconnected {

	override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = for {
		mloc <- matchInstances(firstFullName, unexpanded)
		sloc <- matchInstances(secondFullName, unexpanded)
		if mloc.instance.hasInterface[PortsLike]
		if sloc.instance.hasInterface[PortsLike]
		mi = mloc.instance.getInterfaceUnwrapped[PortsLike]
		si = sloc.instance.getInterfaceUnwrapped[PortsLike]
		fp <- mi.getPortsStartingWith("")
		if firstFullName.split('.').map(_ == fp.name).reduce((a, b) => a | b)
		sp <- si.getPortsStartingWith("")
		if secondFullName.split('.').map(_ == sp.name).reduce((a, b) => a | b)
	} yield
		ConnectedPortGroup(mi, fp, mloc.fullName, si, sp, direction)

	override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

	override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] = Seq()

	override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
