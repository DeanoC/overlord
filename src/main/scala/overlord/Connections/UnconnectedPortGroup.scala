package overlord.Connections

import overlord.Instances.{ChipInstance, InstanceTrait}
import overlord.Interfaces.PortsLike
import overlord._

case class UnconnectedPortGroup(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String,
    first_prefix: String,
    second_prefix: String,
    excludes: Seq[String]
) extends Unconnected {
  override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = for {
    mloc <- matchInstances(firstFullName, unexpanded)
    sloc <- matchInstances(secondFullName, unexpanded)
    if mloc.instance.hasInterface[PortsLike]
    if sloc.instance.hasInterface[PortsLike]
    mi = mloc.instance.getInterfaceUnwrapped[PortsLike]
    si = sloc.instance.getInterfaceUnwrapped[PortsLike]
    fp <- mi.getPortsStartingWith(first_prefix)
    if firstFullName.contains(fp.name)
    sp <- si.getPortsStartingWith(second_prefix)
    if secondFullName.contains(sp.name)
    if fp.name.stripPrefix(first_prefix) == sp.name.stripPrefix(second_prefix)
    if !(excludes.contains(fp.name) || excludes.contains(sp.name))
  } yield ConnectedPortGroup(mi, fp, mloc.fullName, si, sp, direction)

  override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

  override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] =
    Seq()

  override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
