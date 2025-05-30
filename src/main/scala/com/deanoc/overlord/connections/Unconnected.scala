package com.deanoc.overlord.connections

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.hardware._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord._
import com.deanoc.overlord.interfaces._
import com.deanoc.overlord.instances.{
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
      if (nm.nonEmpty) Some(InstanceLoc(c, Option(port).flatten, nm.get)) // Ensure port is not null
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
    // Delegate to the ConnectionParser for creating port connections
    ConnectionParser.parsePortConnection(cbp, fil, sil)
  }
}
