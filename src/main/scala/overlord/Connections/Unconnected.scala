package overlord.Connections

import ikuy_utils.{BigIntV, Utils, Variant}
import overlord.Connections
import overlord.Chip._
import overlord.Instances._

import scala.math.BigDecimal.double2bigDecimal

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

	def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = {
		connectionType match {
			case _: PortConnectionType      => connectPortConnection(unexpanded)
			case _: ClockConnectionType     => connectClockConnection(unexpanded)
			case _: ConstantConnectionType  => connectConstantConnection(unexpanded)
			case _: PortGroupConnectionType => connectPortGroupConnection(unexpanded)
			case _: BusConnectionType       => connectBusConnection(unexpanded)
		}
	}

	def preConnect(unexpanded: Seq[ChipInstance]): Unit = {
		connectionType match {
			case _: BusConnectionType => preConnectBusConnection(unexpanded)
			case _                    =>
		}
	}

	private def matchInstances(name: String,
	                           unexpanded: Seq[ChipInstance]):
	Seq[InstanceLoc] = {
		unexpanded.flatMap(c => {
			val (nm, port) = c.getMatchNameAndPort(name)
			if (nm.nonEmpty) Some(InstanceLoc(c, port, nm.get))
			else c match {
				case container: Container => matchInstances(name, container.chipChildren)
				case _                    => None
			}
		})
	}

	private def connectPortConnection(unexpanded: Seq[ChipInstance]) = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		val cbp = if (mo.length > 1 || so.length > 1)
			WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {mloc <- mo; sloc <- so} yield {
			ConnectPortBetween(cbp, mloc, sloc)
		}
	}

	private def connectClockConnection(unexpanded: Seq[ChipInstance]) = {
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
		val fsloc = InstanceLoc(sil.instance, Some(sport), sil.fullName)

		Connections.ConnectedBetween(PortConnectionType(),
		                             cbp, fmloc, direction, fsloc)
	}

	private def connectConstantConnection(unexpanded: Seq[ChipInstance]) = {
		val to       = matchInstances(second, unexpanded)
		val constant = connectionType.asInstanceOf[ConstantConnectionType]
		val ccp      = if (to.length > 1) WildCardConnectionPriority()
		else ExplicitConnectionPriority()

		for {tloc <- to} yield
			ConnectedConstant(connectionType, ccp,
			                  constant.constant, direction, tloc)
	}

	private def ConnectPortGroupBetween(fi: ChipInstance,
	                                    fp: Port,
	                                    fn: String,
	                                    si: ChipInstance,
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

		val fmloc = InstanceLoc(fi, Some(fport), s"$fn.${fp.name}")
		val fsloc = InstanceLoc(si, Some(sport), s"$fn.${sp.name}")

		Connections.ConnectedBetween(PortConnectionType(),
		                             GroupConnectionPriority(),
		                             fmloc, direction, fsloc)
	}


	private def connectPortGroupConnection(unexpanded: Seq[ChipInstance]) = {
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
		     } yield ConnectPortGroupBetween(mloc.instance,
		                                     fp,
		                                     mloc.fullName,
		                                     sloc.instance,
		                                     sp)
	}

	private def preConnectBusConnection(unexpanded: Seq[ChipInstance]): Unit = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		if (mo.isEmpty) {
			println(s"$main instance can't be found")
		}
		if (so.isEmpty) {
			println(s"$second instance can't be found")
		}

		if (mo.length != 1 || so.length != 1) {
			println(s"connection $main between $second count error")
			Seq[ChipInstance]()
		}

		val mainIL      = mo.head
		val otherIL     = so.head
		val isMainABus  = mo.head.instance.isInstanceOf[BusInstance]

		// main = bus and first to second so main is supplier, other is consumer
		// main = bus and second to first so main is consumer, other is supplier
		// other = bus and first to second so main is consumer, other is supplier
		// other = bus and second to first so main is supplier, other is consumer
		val mainIsSupplier =
			(isMainABus && direction == FirstToSecondConnection()) ||
			(!isMainABus && direction == SecondToFirstConnection())

		if (mainIsSupplier) {
			val bus: BusInstance =
				if (isMainABus) mainIL.instance.asInstanceOf[BusInstance]
				else otherIL.instance.asInstanceOf[BusInstance]

			val other = if (!isMainABus) mainIL.instance else otherIL.instance

			val busBankAlignment = Utils.lookupBigInt(bus.attributes.toMap,
			                                          "bus_bank_alignment",
			                                          1024)

			val connectionSize = Utils.lookupBigInt(attributes,
			                                        key = "size_in_bytes",
			                                        busBankAlignment)


			val size    = other match {
				case bridge: BridgeInstance => Utils.pow2(bridge.addressWindowWidth).toBigInt
				case ram: RamInstance => ram.getSizeInBytes
				case _                => connectionSize
			}
			if(attributes.contains("fixed_address")) {
				val faAttrib = attributes("fixed_address")
				if(faAttrib.isInstanceOf[BigIntV]) {
					val address = Utils.toBigInt(faAttrib)
					bus.addFixedAddressConsumer(other, address, size)
				} else {
					val fixedAddresses = Utils.toArray(faAttrib)
					for(fixedAddress <- fixedAddresses) {
						val fa = Utils.toArray(fixedAddress)
						val address = Utils.toBigInt(fa(0))
						val size    = Utils.toBigInt(fa(1))

						bus.addFixedAddressConsumer(other, address, size)
					}

				}
			} else {
				if(!other.isHardware) bus.addVariableAddressConsumer(other, size)
			}
		}
	}

	private def connectBusConnection(unexpanded: Seq[ChipInstance]): Seq[ConnectedBetween] = {
		val mo = matchInstances(main, unexpanded)
		val so = matchInstances(second, unexpanded)

		if (mo.length != 1 || so.length != 1) {
			println(s"connection $main between $second count error")
			Seq[ChipInstance]()
		}

		val mainIL      = mo.head
		val secondaryIL = so.head
		val isMainBus   = mo.head.instance.isInstanceOf[BusInstance]

		val bus: BusInstance =
			if (isMainBus) mainIL.instance.asInstanceOf[BusInstance]
			else secondaryIL.instance.asInstanceOf[BusInstance]
		val other = if (!isMainBus) mainIL.instance else secondaryIL.instance

		val busLoc = if (isMainBus) mainIL else secondaryIL

		// main = bus and first to second so main is bus, other is consumer
		// main = bus and second to first so main is bus, other is supplier
		// other = bus and first to second so main is supplier, other is bus
		// other = bus and second to first so main is consumer, other is bus
		val isBusSupplier =
			(isMainBus && direction == FirstToSecondConnection()) ||
			(!isMainBus && direction == SecondToFirstConnection())

		if(bus.isHardware || other.isHardware) {
			return Seq(ConnectedBetween(BusConnectionType(),
			                        GroupConnectionPriority(), mainIL,
			                        direction, secondaryIL))
		}

		// if the bus isn't the supplier it needs to present its consumer interface
		// and vice versa
		val busPrefixes = if (isBusSupplier)
			Seq(bus.consumerPrefix.replace("${index}",s"${bus.getFirstIndex(other)}"))
		else bus.supplierPrefixes

		// other the same but in reverse and bridges are special
		// bridges are special
		val otherPrefixes =
			if (other.isInstanceOf[BridgeInstance])
				Utils.lookupStrings(other.attributes,
				                    bus.definition.defType.ident.head,
				                    "bus_")
			else if (isBusSupplier)
				Utils.lookupStrings(other.attributes, "consumer_prefix", "bus_")
			else
				Utils.lookupStrings(other.attributes, "supplier_prefix", "bus_")

		if (busPrefixes == otherPrefixes) {
			for {
				bp <- busPrefixes
				fp <- bus.ports.values.filter(_.name.startsWith(bp))
				sp <- other.ports.values.filter(_.name.startsWith(bp))
				if fp.name == sp.name
			} yield {
				if(isMainBus) ConnectPortGroupBetween(bus, fp, busLoc.fullName, other, sp)
				else ConnectPortGroupBetween(other, sp, busLoc.fullName, bus, fp)
			}
		}
		else {
			for {
				bp <- busPrefixes
				op <- otherPrefixes
				fp <- bus.ports.values.filter(_.name.startsWith(bp))
				sp <- other.ports.values.filter(_.name.startsWith(op))
				if fp.name.stripPrefix(bp) == sp.name.stripPrefix(op)
			} yield {
				if(isMainBus) ConnectPortGroupBetween(bus, fp, busLoc.fullName, other, sp)
				else ConnectPortGroupBetween(other, sp, busLoc.fullName, bus, fp)
			}
		}
	}
}