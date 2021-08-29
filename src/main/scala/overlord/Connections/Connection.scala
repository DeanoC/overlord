package overlord.Connections

import ikuy_utils._
import overlord.Instances.{BusInstance, ChipInstance}
import overlord.{Connections, DefinitionCatalog}

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

case class LogicalConnectionType() extends ConnectionType

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

	def apply(connection: Variant,
	          catalogs: DefinitionCatalog): Option[Connection] = {
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
		Some(Connections.Unconnected(
			toConnectionType(first, conntype, table),
			first, dir, secondary, table))
	}

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
				Utils.lookupArray(table, "excludes").toSeq.map(Utils.toString))
			case "bus"        => BusConnectionType()
			case "logical"    => LogicalConnectionType()
			case _            =>
				println(s"$ctype is an unknown connection type")
				PortConnectionType()
		}
	}

	def connect(unconnected: Seq[Connection],
	            unexpanded: Seq[ChipInstance]): Seq[Connection] =
		(for (c <- unconnected.filter(_.isUnconnected).map(_.asUnconnected)) yield {
			c.connect(unexpanded)
		}).flatten

	def preConnect(unconnected: Seq[Connection],
	               unexpanded: Seq[ChipInstance]): Unit = {
		for (c <- unconnected.filter(_.isUnconnected).map(_.asUnconnected))
			c.preConnect(unexpanded)

		unexpanded.filter(_.isInstanceOf[BusInstance])
			.map(_.asInstanceOf[BusInstance])
			.foreach(_.computeConsumerAddresses())

	}
}