package overlord.Connections

import overlord.Connections
import overlord.Gateware.{Port, WireDirection}
import overlord.Instances.{Container, Instance}

trait UnconnectedTrait extends Connection {
	def first: String

	def second: String

	def connect(unexpanded: Seq[Instance]): Seq[Connected]
}

case class Unconnected(connectionType: ConnectionType,
                       main: String,
                       direction: ConnectionDirection,
                       secondary: String,
                      ) extends UnconnectedTrait {
	override def firstFullName: String = main

	override def secondFullName: String = secondary

	override def first: String = main

	override def second: String = secondary

	override def connect(unexpanded: Seq[Instance]): Seq[Connected] = {
		connectionType match {
			case _: PortConnectionType      => ConnectPortConnection(unexpanded)
			case _: ClockConnectionType     => ConnectPortConnection(unexpanded)
			case _: ConstantConnectionType  => ConnectConstantConnection(unexpanded)
			case _: PortGroupConnectionType => ConnectPortGroupConnection(unexpanded)
		}
	}

	private def matchInstances(name: String,
	                           unexpanded: Seq[Instance]):
	Seq[InstanceLoc] = {
		unexpanded.flatMap(c => {
			val (nm, port) = c.getMatchNameAndPort(name)
			if (nm.nonEmpty) Some(InstanceLoc(c, port, nm.get))
			else c match {
				case container: Container => matchInstances(name, container.children)
				case _                    => None
			}
		})
	}

	private def ConnectPortConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		val cbp = if (mo.length > 1 || so.length > 1)
			WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {mloc <- mo; sloc <- so} yield
			ConnectedBetween(connectionType, cbp, mloc, direction, sloc)
	}

	private def ConnectConstantConnection(unexpanded: Seq[Instance]) = {
		val to       = matchInstances(second, unexpanded)
		val constant = connectionType.asInstanceOf[ConstantConnectionType]
		val ccp      = if (to.length > 1) WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {tloc <- to} yield
			ConnectedConstant(connectionType, ccp,
			                  constant.constant, direction, tloc)
	}

	private def ConnectPortGroupConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		val ct = connectionType.asInstanceOf[PortGroupConnectionType]

		for {mloc <- mo; sloc <- so
		     fp <- mloc.instance.phase2Ports.values
			     .filter(_.name.startsWith(ct.first_prefix))
		     sp <- sloc.instance.phase2Ports.values
			     .filter(_.name.startsWith(ct.second_prefix))
		     if fp.name.stripPrefix(ct.first_prefix) ==
		        sp.name.stripPrefix(ct.second_prefix)
		     if !(ct.excludes.contains(fp.name) || ct.excludes.contains(sp.name))
		     } yield {
			val fmloc = InstanceLoc(mloc.instance,
			                        Some(fp),
			                        s"${mloc.fullName}.${fp.name}")
			val fsloc = InstanceLoc(sloc.instance,
			                        Some(sp),
			                        s"${mloc.fullName}.${sp.name}")

			Connections.ConnectedBetween(PortConnectionType(),
			                             GroupConnectionPriority(),
			                             fmloc, direction, fsloc)
		}
	}
}