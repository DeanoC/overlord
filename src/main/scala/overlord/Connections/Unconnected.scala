package overlord.Connections

import overlord.Connections
import overlord.Instances.Instance

trait UnconnectedTrait extends Connection {
	def first: String

	def second: String

	def connect(unexpanded: Seq[Instance]): Seq[Connected]
}

case class Unconnected(connectionType: ConnectionType,
                       main: String,
                       secondary: String,
                      ) extends UnconnectedTrait {
	override def firstFullName: String = main

	override def secondFullName: String = secondary

	override def first: String = main

	override def second: String = secondary

	override def connect(unexpanded: Seq[Instance]): Seq[Connected] = {
		connectionType match {
			case PortConnectionType() | ClockConnectionType()
			                               => ConnectPortConnection(unexpanded)
			case ConstantConnectionType()  => ConnectConstantConnection(unexpanded)
			case PortGroupConnectionType() => ConnectPortGroupConnection(unexpanded)
		}
	}


	private def matchInstances(name: String, unexpanded: Seq[Instance]) = {
		unexpanded.flatMap(c => {
			val found = c.getMatchName(name)
			if (found.nonEmpty) Some(found.get, c)
			else None
		})
	}

	private def ConnectPortConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		for {(mnm, m) <- mo
		     (snm, s) <- so} yield
			ConnectedBetween(connectionType,
			                 if (mo.length > 1 || so.length > 1)
				                 WildCardConnectionPriority()
			                 else ExplicitConnectionPriority(),
			                 m, s, mnm, snm)
	}

	private def ConnectConstantConnection(unexpanded: Seq[Instance]) = {
		val so = matchInstances(second, unexpanded)
		for {(snm, s) <- so} yield
			ConnectedConstant(connectionType,
			                  if (so.length > 1) WildCardConnectionPriority()
			                  else ExplicitConnectionPriority(),
			                  first, s, snm)
	}

	private def ConnectPortGroupConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		for {(mnm, m) <- mo
		     (snm, s) <- so
		     fp <- m.definition.defType.portsOrParameters
		     sp <- s.definition.defType.portsOrParameters
		     if fp == sp
		     } yield
			Connections.ConnectedBetween(PortConnectionType(),
			                             GroupConnectionPriority(),
			                             m, s,
			                             s"${mnm}.$fp",
			                             s"${snm}.$sp")
	}
}