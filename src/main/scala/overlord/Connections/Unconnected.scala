package overlord.Connections

import ikuy_utils.{Utils, Variant}
import overlord.Chip._
import overlord.DefinitionCatalog
import overlord.Instances._
import overlord.Interfaces.{QueryInterface, UnConnectedLike}

trait UnConnected extends QueryInterface with UnConnectedLike {

	def direction: ConnectionDirection

	def firstFullName: String

	def secondFullName: String

	protected def matchInstances(nameToMatch: String, unexpanded: Seq[ChipInstance]): Seq[InstanceLoc] = {
		unexpanded.flatMap(c => {
			val (nm, port) = c.getMatchNameAndPort(nameToMatch)
			if (nm.nonEmpty) Some(InstanceLoc(c, port, nm.get))
			else c match {
				case container: Container => matchInstances(nameToMatch, container.chipChildren)
				case _                    => None
			}
		})
	}

	protected def ConnectPortBetween(cbp: ConnectionPriority,
	                                 fil: InstanceLoc,
	                                 sil: InstanceLoc): ConnectedPortGroup = {
		val fp = {
			if (fil.port.nonEmpty) fil.port.get
			else if (fil.isPin) {
				fil.instance.asInstanceOf[PinGroupInstance].constraint.ports.head
			} else if (fil.isClock) {
				Port(fil.fullName, BitsDesc(1), InWireDirection())
			} else {
				if (fil.isGateware) println(s"${fil.fullName} unable to get port")
				Port(fil.fullName, BitsDesc(1), InWireDirection())
			}
		}
		val sp = {
			if (sil.port.nonEmpty) sil.port.get
			else if (sil.isPin) {
				sil.instance.asInstanceOf[PinGroupInstance].constraint.ports.head
			} else if (sil.isClock) {
				Port(sil.fullName, BitsDesc(1), OutWireDirection())
			} else {
				if (sil.isGateware) println(s"${sil.fullName} unable to get port")
				Port(sil.fullName, fp.width, InWireDirection())
			}
		}

		var firstDirection  = fp.direction
		var secondDirection = sp.direction

		if (fp.direction != InOutWireDirection()) {
			if (sp.direction == InOutWireDirection()) secondDirection = fp.direction
		} else if (sp.direction != InOutWireDirection()) firstDirection = sp.direction

		val fport = fp.copy(direction = firstDirection)
		val sport = sp.copy(direction = secondDirection)

		val fmloc = InstanceLoc(fil.instance, Some(fport), fil.fullName)
		val fsloc = InstanceLoc(sil.instance, Some(sport), sil.fullName)

		ConnectedPortGroup(cbp, fmloc, direction, fsloc)
	}
}

object Unconnected {

	def apply(connection: Variant,
	          catalogs: DefinitionCatalog): Option[UnConnectedLike] = {
		val table = Utils.toTable(connection)

		if (!table.contains("type")) {
			println(s"connection $connection requires a type field")
			return None
		}
		val conntype = Utils.toString(table("type"))

		if (!table.contains("connection")) {
			println(s"connection $conntype requires a connection field")
			return None
		}

		val cons = Utils.toString(table("connection"))
		val con  = cons.split(' ')
		if (con.length != 3) {
			println(s"$conntype has an invalid connection field: $cons")
			return None
		}
		val (first, dir, secondary) = con(1) match {
			case "->"         => (con(0), FirstToSecondConnection(), con(2))
			case "<->" | "<>" => (con(0), BiDirectionConnection(), con(2))
			case "<-"         => (con(0), SecondToFirstConnection(), con(2))
			case _            =>
				println(s"$conntype has an invalid connection ${con(1)} : $cons")
				return None
		}


		conntype match {
			case "port"       => Some(UnConnectedPortGroup(first, dir, secondary, "", "", Seq()))
			case "clock"      => Some(UnConnectedClock(first, dir, secondary))
			case "constant"   => Some(UnConnectedConstant(first, dir, secondary, Utils.stringToVariant(first)))
			case "port_group" => Some(UnConnectedPortGroup(first, dir, secondary,
			                                               Utils.lookupString(table, "first_prefix", ""),
			                                               Utils.lookupString(table, "second_prefix", ""),
			                                               Utils.lookupArray(table, "excludes").toSeq.map(Utils.toString)))
			case "bus"        => {
				Some(UnConnectedBus(first,
				                    dir,
				                    secondary,
				                    Utils.lookupString(table, "bus_protocol", "internal"),
				                    Utils.lookupString(table, "bus_name", ""),
				                    Utils.lookupBoolean(table, "silent", or = false)
				                    ))
			}
			case "logical"    => Some(UnConnectedLogical(first, dir, secondary))
			case _            =>
				println(s"$conntype is an unknown connection type")
				None
			//				PortConnectionType()
		}
	}
}