package overlord.Connections

import overlord.Gateware.{
	InOutWireDirection, InWireDirection,
	OutWireDirection, WireDirection
}
import overlord.Instances.Instance
import overlord.{Connections, DefinitionCatalog}
import ikuy_utils._
import toml.Value

import scala.collection.mutable

sealed trait ConnectionDirection {
	override def toString: String =
		this match {
			case FirstToSecondConnection() => "first to second"
			case SecondToFirstConnection() => "second to first"
			case BiDirectionConnection()   => "bi direction"
		}

	def flip: ConnectionDirection = this match {
		case FirstToSecondConnection() => SecondToFirstConnection()
		case SecondToFirstConnection() => FirstToSecondConnection()
		case BiDirectionConnection()   => this
	}

}

case class FirstToSecondConnection() extends ConnectionDirection

case class SecondToFirstConnection() extends ConnectionDirection

case class BiDirectionConnection() extends ConnectionDirection

sealed trait ConnectionType

case class PortConnectionType() extends ConnectionType

case class ClockConnectionType() extends ConnectionType

case class ConstantConnectionType(constant: Variant) extends ConnectionType

case class PortGroupConnectionType(first_prefix: String,
                                   second_prefix: String,
                                   excludes: Seq[String])
	extends ConnectionType

case class BusConnectionType() extends ConnectionType

trait Connection {

	val connectionType: ConnectionType

	def direction: ConnectionDirection

	def isUnconnected: Boolean = this.isInstanceOf[Unconnected]

	def asUnconnected: Unconnected = this.asInstanceOf[Unconnected]

	def isConnected: Boolean = this.isInstanceOf[Connected]

	def asConnected: Connected = this.asInstanceOf[Connected]

	def firstFullName: String

	def secondFullName: String
}

object Connection {

	def toConnectionType(first: String,
	                     ctype: String,
	                     table: Map[String, Variant]): ConnectionType = {
		ctype match {
			case "port"       => PortConnectionType()
			case "clock"      => ClockConnectionType()
			case "constant"   => ConstantConnectionType(Utils.stringToVariant(first))
			case "port_group" => PortGroupConnectionType(
				Utils.lookupString(table, "first_prefix", ""),
				Utils.lookupString(table, "second_prefix", ""),
				Utils.lookupArray(table, "excludes").map(Utils.toString))
			case "bus"        => BusConnectionType()
			case _            =>
				println(s"$ctype is an unknown connection type")
				PortConnectionType()
		}
	}

	def apply(connection: Variant,
	          catalogs: DefinitionCatalog): Option[Connection] = {
		val table = Utils.toTable(connection)

		if (!table.contains("type")) {
			println(s"connection ${connection} requires a type field")
			return None
		}
		val conntype = Utils.toString(table("type"))

		if (!table.contains("connection")) {
			println(s"connection ${conntype} requires a connection field")
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
		Some(Connections.Unconnected(
			toConnectionType(first, conntype, table),
			first, dir, secondary, table))
	}

	private def connect(unconnected: Seq[Connection],
	                    unexpanded: Seq[Instance]): Seq[Connection] = {
		(for (c <- unconnected.filter(_.isUnconnected).map(_.asUnconnected)) yield
			c.connect(unexpanded)).flatten
	}

	def preConnect(unconnected: Seq[Connection],
	               unexpanded: Seq[Instance]): Unit = {
		(for (c <- unconnected.filter(_.isUnconnected).map(_.asUnconnected))
			c.preConnect(unexpanded))
	}

	private def expand(instances: Seq[Instance],
	                   unexpanded: Seq[Instance]): Seq[Instance] =
		(for {
			inst <- instances
			if inst.replicationCount > 1
			index <- 0 until inst.replicationCount
		} yield inst.copyMutate(s"${inst.ident}.$index")) ++
		unexpanded.diff(instances)


	private def expandConnections(expanding: Seq[Connected],
	                              expanded: Seq[Instance],
	                              connected: Seq[Connection]):
	Seq[Connection] = {
		val result = mutable.ArrayBuffer[Connection]()
		val done   = mutable.ArrayBuffer[Connection]()

		for {
			con <- expanding
			if con.areConnectionCountsCompatible
			if !done.contains(con)
			i <- 0 until 2
		} {
			if (i == 1) done += con
			// expansion of connections requires equal counts on both sides
			// OR one side to be shared (NxMConnect are always shared)
			// we also have to ensure we dont double count when replicated both
			// sides
			val doReplicate = (i == 0 && con.first.nonEmpty &&
			                   con.firstCount == con.secondaryCount) ||
			                  ((con.firstCount != con.secondaryCount) &&
			                   ((i == 0 && con.firstCount != 1 &&
			                     con.first.nonEmpty) ||
			                    (i == 1 && con.secondaryCount != 1 &&
			                     con.second.nonEmpty)))

			if (doReplicate) {

				val count = (if (i == 0) con.first.get else con.second.get)
					.asInstanceOf[Instance].replicationCount

				for (index <- 0 until count) yield {
					val m: Instance = {
						val mident = if (con.firstCount == 1) con.firstFullName
						else s"${con.firstFullName}.$index"
						(expanded.find(p => p.ident == s"$mident") match {
							case Some(value) => value
							case None        => println(s"$mident isn't a instance name")
								return Seq()
						})
					}

					val s: Instance = {
						val sident = if (con.secondaryCount == 1) con.secondFullName
						else s"${con.secondFullName}.${index}"
						(expanded.find(p => p.ident == s"$sident") match {
							case Some(value) => value
							case None        => println(s"$sident isn't a instance name")
								return Seq()
						})
					}

					result += (con match {
						case ConnectedBetween(t, _, om, d, os) =>
							Connections.ConnectedBetween(
								t,
								WildCardConnectionPriority(),
								InstanceLoc(m, om.port, m.ident),
								d,
								InstanceLoc(s, os.port, s.ident))
						case ConnectedConstant(t, _, c, d, ot) =>
							Connections.ConnectedConstant(
								t,
								WildCardConnectionPriority(),
								c,
								d,
								InstanceLoc(s, ot.port, s.ident))
						case v                                 =>
							println(s"Expansion of unknown Connected Type?? $con")
							v
					})
				}
			}
		}

		(result ++ connected.diff(result))
			.filter(!_.connectionType.isInstanceOf[PortGroupConnectionType])
			.toSeq
	}


	def expandAndConnect(unconnected: Seq[Connection],
	                     unexpanded: Seq[Instance]):
	((Seq[Instance], Seq[Connection])) = {
		val connected = Connection.connect(unconnected, unexpanded)

		val expandableInstances = unexpanded.filter(_.replicationCount > 1)

		val connectionsNeedExpanding = for {
			toExpand <- expandableInstances
			con <- connected.filter(_.isConnected).map(_.asConnected)
			if con.connectsToInstance(toExpand)
		} yield con

		val expanded = Connection.expand(expandableInstances, unexpanded)

		val expandedConnection = Connection.expandConnections(
			connectionsNeedExpanding,
			expanded,
			connected)

		val dupsseq = for (o <- expandedConnection) yield
			expandedConnection.filter(c => o.firstFullName == c.firstFullName &&
			                               o.secondFullName == c.secondFullName)

		val dupToUse    = (for (dups <- dupsseq) yield {

			val expli = dups.find(_.asConnected
				                      .connectionPriority
				                      .isInstanceOf[ExplicitConnectionPriority])
			val wildc = dups.find(_.asConnected
				                      .connectionPriority
				                      .isInstanceOf[WildCardConnectionPriority])
			val grpc  = dups.find(_.asConnected
				                      .connectionPriority
				                      .isInstanceOf[GroupConnectionPriority])
			if (expli.nonEmpty) expli.get
			else if (wildc.nonEmpty) wildc.get
			else if (grpc.nonEmpty) grpc.get
			else dups.head
		})
		val connections = expandedConnection.diff(dupsseq) ++ dupToUse

		(expanded, connections)
	}

}