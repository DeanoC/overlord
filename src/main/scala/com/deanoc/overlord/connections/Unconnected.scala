package com.deanoc.overlord.Connections

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.Hardware._
import com.deanoc.overlord.ConnectionDirection
import com.deanoc.overlord._
import com.deanoc.overlord.Interfaces._
import com.deanoc.overlord.Instances.{
  InstanceTrait,
  PinGroupInstance,
  Container
}

/**
  * Trait representing unconnected components in the system.
  *
  * Provides methods for matching instances and creating connections between
  * components that are not yet connected.
  */
trait Unconnected extends QueryInterface with UnconnectedLike {

  /**
    * Matches instances based on a name and returns their locations.
    *
    * @param nameToMatch
    *   The name to match against.
    * @param unexpanded
    *   A sequence of unexpanded instances.
    * @return A sequence of matched instance locations.
    */
  protected def matchInstances(
      nameToMatch: String,
      unexpanded: Seq[InstanceTrait]
  ): Seq[InstanceLoc] = {
    unexpanded.flatMap(c => {
      val (nm, port) = c.getMatchNameAndPort(nameToMatch)
      if (nm.nonEmpty) Some(InstanceLoc(c, port, nm.get))
      else
        c match {
          case container: Container =>
            matchInstances(nameToMatch, container.chipChildren)
          case _ => None
        }
    })
  }

  /**
    * Connects two ports between two instance locations.
    *
    * @param cbp
    *   The connection priority.
    * @param fil
    *   The first instance location.
    * @param sil
    *   The second instance location.
    * @return A `ConnectedPortGroup` representing the connection.
    */
  protected def ConnectPortBetween(
      cbp: ConnectionPriority,
      fil: InstanceLoc,
      sil: InstanceLoc
  ): ConnectedPortGroup = {
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

    var firstDirection = fp.direction
    var secondDirection = sp.direction

    if (fp.direction != InOutWireDirection()) {
      if (sp.direction == InOutWireDirection()) secondDirection = fp.direction
    } else if (sp.direction != InOutWireDirection())
      firstDirection = sp.direction

    val fport = fp.copy(direction = firstDirection)
    val sport = sp.copy(direction = secondDirection)

    val fmloc = InstanceLoc(fil.instance, Some(fport), fil.fullName)
    val fsloc = InstanceLoc(sil.instance, Some(sport), sil.fullName)

    ConnectedPortGroup(cbp, fmloc, direction, fsloc)
  }
}

/** Factory object for creating instances of `Unconnected`. */
object Unconnected {

  /**
    * Creates an `UnconnectedLike` instance based on the provided connection variant.
    *
    * @param connection
    *   The connection variant.
    * @return An optional `UnconnectedLike` instance.
    */
  def apply(connection: Variant): Option[UnconnectedLike] = {
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
    val con = cons.split(' ')
    if (con.length != 3) {
      println(s"$conntype has an invalid connection field: $cons")
      return None
    }
    val (first, dir, secondary) = con(1) match {
      case "->"         => (con(0), FirstToSecondConnection(), con(2))
      case "<->" | "<>" => (con(0), BiDirectionConnection(), con(2))
      case "<-"         => (con(0), SecondToFirstConnection(), con(2))
      case _ =>
        println(s"$conntype has an invalid connection ${con(1)} : $cons")
        return None
    }

    conntype match {
      case "port"  => Some(UnconnectedPort(first, dir, secondary))
      case "clock" => Some(UnconnectedClock(first, dir, secondary))
      case "parameters" =>
        if (first != "_") None
        else
          Some(
            UnconnectedParameters(
              dir,
              secondary,
              Utils.lookupArray(table, "parameters")
            )
          )
      case "port_group" =>
        Some(
          UnconnectedPortGroup(
            first,
            dir,
            secondary,
            Utils.lookupString(table, "first_prefix", ""),
            Utils.lookupString(table, "second_prefix", ""),
            Utils.lookupArray(table, "excludes").toSeq.map(Utils.toString)
          )
        )
      case "bus" => {
        val supplierBusName = Utils.lookupString(table, "bus_name", "")
        val consumerBusName =
          Utils.lookupString(table, "consumer_bus_name", supplierBusName)
        Some(
          UnconnectedBus(
            first,
            dir,
            secondary,
            Utils.lookupString(table, "bus_protocol", "internal"),
            supplierBusName,
            consumerBusName,
            Utils.lookupBoolean(table, "silent", or = false)
          )
        )
      }
      case "logical" => Some(UnconnectedLogical(first, dir, secondary))
      case _ =>
        println(s"$conntype is an unknown connection type")
        None
    }
  }
}
