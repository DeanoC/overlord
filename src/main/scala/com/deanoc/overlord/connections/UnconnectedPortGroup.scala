package com.deanoc.overlord.connections

import com.deanoc.overlord.instances.{HardwareInstance, InstanceTrait}
import com.deanoc.overlord.interfaces.PortsLike
import com.deanoc.overlord._
import com.deanoc.overlord.connections.ConnectionDirection

/**
  * Represents an unconnected port group connection between two components.
  *
  * This case class defines the properties and methods for managing
  * unconnected port group connections, including establishing connections
  * and collecting constants.
  *
  * @param firstFullName
  *   The full name of the first component in the connection.
  * @param direction
  *   The direction of the connection.
  * @param secondFullName
  *   The full name of the second component in the connection.
  * @param first_prefix
  *   The prefix for ports in the first component. Default empty string for direct port connections.
  * @param second_prefix
  *   The prefix for ports in the second component. Default empty string for direct port connections.
  * @param excludes
  *   A sequence of port names to exclude from the connection. Default empty sequence for no exclusions.
  * @param matchPrefixStripped
  *   Whether to match ports by stripping prefixes and comparing suffixes. Default true for port groups.
  */
case class UnconnectedPortGroup(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String,
    first_prefix: String = "",
    second_prefix: String = "",
    excludes: Seq[String] = Seq.empty
) extends Unconnected {

  /**
    * Establishes the port group connection between components.
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    * @return
    *   A sequence of connected port groups.
    */
  override def connect(unexpanded: Seq[HardwareInstance]): Seq[Connected] = for {
    mloc <- matchInstances(firstFullName, unexpanded)
    sloc <- matchInstances(secondFullName, unexpanded)
    mi <- mloc.instance.getInterface[PortsLike].toSeq
    si <- sloc.instance.getInterface[PortsLike].toSeq
    fp <- mi.getPortsStartingWith(first_prefix).filter(p => firstFullName.contains(p.name))
    sp <- si.getPortsStartingWith(second_prefix).filter(p => secondFullName.contains(p.name))

    if (fp.name.stripPrefix(first_prefix) == sp.name.stripPrefix(second_prefix)) &&
       !excludes.contains(fp.name) && !excludes.contains(sp.name)
  } yield ConnectedPortGroup(mi, fp, mloc.fullName, si, sp, direction)

  /**
    * Performs pre-connection checks for the port group connection (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def preConnect(unexpanded: Seq[HardwareInstance]): Unit = None

  /**
    * Collects constants associated with the unconnected port group.
    *
    * @param unexpanded
    *   A sequence of unexpanded instance traits.
    * @return
    *   A sequence of constants (empty for this implementation).
    */
  override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] =
    Seq()

  /**
    * Finalizes the port group connection (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def finaliseBuses(unexpanded: Seq[HardwareInstance]): Unit = None
}
