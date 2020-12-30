package overlord.Connections

import ikuy_utils.{Utils, Variant}
import overlord.Connections
import overlord.Gateware._
import overlord.Instances._

case class Unconnected(connectionType: ConnectionType,
                       main: String,
                       direction: ConnectionDirection,
                       secondary: String,
                       attributes: Map[String, Variant]
                      ) extends Connection {
	def firstFullName: String = main

	def secondFullName: String = secondary

	def first: String = main

	def second: String = secondary

	def isConstant: Boolean = connectionType match {
		case ConstantConnectionType(_) => true
		case _                         => false
	}

	def connect(unexpanded: Seq[Instance]): Seq[Connected] = {
		connectionType match {
			case _: PortConnectionType      => connectPortConnection(unexpanded)
			case _: ClockConnectionType     => connectClockConnection(unexpanded)
			case _: ConstantConnectionType  => connectConstantConnection(unexpanded)
			case _: PortGroupConnectionType => connectPortGroupConnection(unexpanded)
			case _: BusConnectionType       => connectBusConnection(unexpanded)
		}
	}

	def preConnect(unexpanded: Seq[Instance]): Unit = {
		connectionType match {
			case _: BusConnectionType => preConnectBusConnection(unexpanded)
			case _                    =>
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

	private def connectPortConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		val cbp = if (mo.length > 1 || so.length > 1)
			WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {mloc <- mo; sloc <- so} yield {
			ConnectPortBetween(cbp, mloc, sloc)
		}
	}

	private def connectClockConnection(unexpanded: Seq[Instance]) = {
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

	private def connectConstantConnection(unexpanded: Seq[Instance]) = {
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


	private def connectPortGroupConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		val ct = connectionType.asInstanceOf[PortGroupConnectionType]

		for {mloc <- mo; sloc <- so
		     fp <- mloc.instance.ports.values
			     .filter(_.name.startsWith(ct.first_prefix))
		     sp <- sloc.instance.ports.values
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

	private def preConnectBusConnection(unexpanded: Seq[Instance]): Unit = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		if (mo.length != 1 || so.length != 1) {
			println(s"connection $main between $second count error")
			Seq[Instance]()
		}

		val mainIL      = mo.head
		val secondaryIL = so.head
		val isMainBus   = mo.head.instance.isInstanceOf[BusInstance]

		// main = bus and first to second so main is supplier, other is consumer
		// main = bus and second to first so main is consumer, other is supplier
		// other = bus and first to second so main is consumer, other is supplier
		// other = bus and second to first so main is supplier, other is consumer
		val mainIsSupplier =
			(isMainBus && direction == FirstToSecondConnection()) ||
			(!isMainBus && direction == SecondToFirstConnection())

		if (mainIsSupplier) {
			val bus: BusInstance = if (isMainBus)
				mainIL.instance.asInstanceOf[BusInstance]
			else
				secondaryIL.instance.asInstanceOf[BusInstance]

			val other = if (!isMainBus) mainIL.instance
			else secondaryIL.instance

			val busBankSize = Utils.lookupBigInt(bus.parameters,
			                                     "bus_bank_size",
			                                     1024)

			val connectionSize = Utils.lookupBigInt(attributes,
			                                        key = "size",
			                                        busBankSize)


			val size    = other match {
				case ram: RamInstance =>
					ram.sizeInBytes match {
						case Some(v) => v
						case None    =>
							Utils.lookupBigInt(ram.definition.attributes,
							                   "size_in_bytes",
							                   connectionSize)
					}
				case _                => connectionSize
			}
			val address = Utils.lookupBigInt(
				attributes,
				key = "address", {
					val base = Utils.lookupBigInt(bus.parameters, "base_bus", 0)
					base + bus.consumerCount * size
				})

			bus.addConsumer(other, address, size)
		}
	}

	private def connectBusConnection(unexpanded: Seq[Instance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		if (mo.length != 1 || so.length != 1) {
			println(s"connection $main between $second count error")
			Seq[Instance]()
		}

		val mainIL      = mo.head
		val secondaryIL = so.head
		val isMainBus   = mo.head.instance.isInstanceOf[BusInstance]

		// main = bus and first to second so main is supplier, other is consumer
		// main = bus and second to first so main is consumer, other is supplier
		// other = bus and first to second so main is consumer, other is supplier
		// other = bus and second to first so main is supplier, other is consumer
		val mainIsSupplier =
			(isMainBus && direction == FirstToSecondConnection()) ||
			(!isMainBus && direction == SecondToFirstConnection())

		val busLoc = if (isMainBus) mainIL else secondaryIL

		val bus: BusInstance = if (isMainBus)
			mainIL.instance.asInstanceOf[BusInstance]
		else
			secondaryIL.instance.asInstanceOf[BusInstance]

		val other = if (!isMainBus) mainIL.instance
		else secondaryIL.instance

		val busPrefixes = if (mainIsSupplier) {
			Seq(
				bus.consumerPrefix.replace("${index}",
				                           s"${bus.getIndex(other)}")
				)
		} else bus.supplierPrefixes

		// bridges are special
		val otherPrefixes = (if (other.isInstanceOf[BridgeInstance]) {
			Utils.lookupStrings(other.definition.attributes,
			                    bus.definition.defType.ident.head,
			                    "bus_")
		} else {
			if (mainIsSupplier) {
				Utils.lookupStrings(other.definition.attributes,
				                    "consumer_prefix",
				                    "bus_")
			} else {
				Utils.lookupStrings(other.definition.attributes,
				                    "supplier_prefix",
				                    "bus_")
			}
		})

		if (busPrefixes == otherPrefixes) for {
			bp <- busPrefixes
			fp <- bus.ports.values.filter(_.name.startsWith(bp))
			sp <- other.ports.values.filter(_.name.startsWith(bp))
			if fp.name == sp.name
		} yield
			ConnectPortGroupBetween(bus, fp, busLoc.fullName, other, sp)
		else for {
			bp <- busPrefixes
			op <- otherPrefixes
			fp <- bus.ports.values.filter(_.name.startsWith(bp))
			sp <- other.ports.values.filter(_.name.startsWith(op))
			if fp.name.stripPrefix(bp) == sp.name.stripPrefix(op)
		} yield
			ConnectPortGroupBetween(bus, fp, busLoc.fullName, other, sp)

	}

}