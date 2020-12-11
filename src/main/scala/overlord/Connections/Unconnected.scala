package overlord.Connections

import overlord.{Connections, DiffPinConstraint, PinConstraint}
import overlord.Gateware.{
	BitsDesc, InOutWireDirection, InWireDirection,
	OutWireDirection, Port, WireDirection
}
import overlord.Instances.{Container, Instance, PinGroupInstance}

trait UnconnectedTrait extends Connection {
	def first: String

	def second: String

	def isConstant: Boolean

	def connect(unexpanded: Seq[Instance]): Seq[Connected]
}

case class Unconnected(connectionType: ConnectionType,
                       main: String,
                       direction: ConnectionDirection,
                       secondary: String,
                      ) extends UnconnectedTrait {
	def firstFullName: String = main

	def secondFullName: String = secondary

	override def first: String = main

	override def second: String = secondary

	override def isConstant: Boolean = connectionType match {
		case ConstantConnectionType(_) => true
		case _                         => false
	}

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

		for {mloc <- mo; sloc <- so} yield {
			ConnectPortBetween(cbp, mloc, sloc)
		}
	}

	private def ConnectPortBetween(cbp: ConnectionPriority,
	                               fil: InstanceLoc,
	                               sil: InstanceLoc) = {
		val fp = {
			if (fil.port.nonEmpty) fil.port.get
			else if (fil.isPin) {
				fil.instance.asInstanceOf[PinGroupInstance].constraint.ports.head
			} else if (fil.isClock) {
				Port(fil.fullName, BitsDesc(1), InWireDirection())
			} else {
				println(s"${fil.fullName} unable to get port")
				Port(fil.fullName, BitsDesc(1), InOutWireDirection())
			}
		}
		val sp = {
			if (sil.port.nonEmpty) sil.port.get
			else if (sil.isPin) {
				sil.instance.asInstanceOf[PinGroupInstance].constraint.ports.head
			} else if (sil.isClock) {
				Port(sil.fullName, BitsDesc(1), InWireDirection())
			} else {
				println(s"${sil.fullName} unable to get port")
				Port(sil.fullName, BitsDesc(1), InOutWireDirection())
			}
		}

		var firstDirection  = fp.direction
		var secondDirection = sp.direction

		if (fp.direction != InOutWireDirection()) {
			if (sp.direction == InOutWireDirection())
				secondDirection = fp.direction
		} else {
			if (sp.direction != InOutWireDirection())
				firstDirection = sp.direction
		}

		val fport = fp.copy(direction = firstDirection)
		val sport = sp.copy(direction = secondDirection)

		val fmloc = InstanceLoc(fil.instance, Some(fport), fil.fullName)
		val fsloc = InstanceLoc(sil.instance, Some(sport), fil.fullName)

		Connections.ConnectedBetween(PortConnectionType(),
		                             cbp, fmloc, direction, fsloc)
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

	private def ConnectPortGroupBetween(fi: Instance,
	                                    fp: Port,
	                                    fn: String,
	                                    si: Instance,
	                                    sp: Port) = {
		var firstDirection  = fp.direction
		var secondDirection = sp.direction

		if (fp.direction != InOutWireDirection()) {
			if (sp.direction == InOutWireDirection())
				secondDirection = fp.direction
		} else {
			if (sp.direction != InOutWireDirection())
				firstDirection = sp.direction
		}

		val fport = fp.copy(direction = firstDirection)
		val sport = sp.copy(direction = secondDirection)

		val fmloc = InstanceLoc(fi, Some(fport), s"${fn}.${fp.name}")
		val fsloc = InstanceLoc(si, Some(sport), s"${fn}.${sp.name}")

		Connections.ConnectedBetween(PortConnectionType(),
		                             GroupConnectionPriority(),
		                             fmloc, direction, fsloc)
	}


	private def ConnectPortGroupConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		val ct = connectionType.asInstanceOf[PortGroupConnectionType]

		for {mloc <- mo; sloc <- so
		     fp <- mloc.instance.getPorts.values
			     .filter(_.name.startsWith(ct.first_prefix))
		     sp <- sloc.instance.getPorts.values
			     .filter(_.name.startsWith(ct.second_prefix))
		     if fp.name.stripPrefix(ct.first_prefix) ==
		        sp.name.stripPrefix(ct.second_prefix)
		     if !(ct.excludes.contains(fp.name) || ct.excludes.contains(sp.name))
		     } yield {
			ConnectPortGroupBetween(mloc.instance,
			                        fp,
			                        mloc.fullName,
			                        sloc.instance,
			                        sp)
		}
	}
}