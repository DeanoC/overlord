package com.deanoc.overlord.Connections

import com.deanoc.overlord.Instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.Interfaces.PortsLike
import com.deanoc.overlord._

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
  *   The prefix for ports in the first component.
  * @param second_prefix
  *   The prefix for ports in the second component.
  * @param excludes
  *   A sequence of port names to exclude from the connection.
  */
case class UnconnectedPortGroup(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String,
    first_prefix: String,
    second_prefix: String,
    excludes: Seq[String]
) extends Unconnected {

  /**
    * Establishes the port group connection between components.
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    * @return
    *   A sequence of connected port groups.
    */
  override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = for {
    mloc <- matchInstances(firstFullName, unexpanded)
    sloc <- matchInstances(secondFullName, unexpanded)
    if mloc.instance.hasInterface[PortsLike]
    if sloc.instance.hasInterface[PortsLike]
    mi = mloc.instance.getInterfaceUnwrapped[PortsLike]
    si = sloc.instance.getInterfaceUnwrapped[PortsLike]
    fp <- mi.getPortsStartingWith(first_prefix)
    if firstFullName.contains(fp.name)
    sp <- si.getPortsStartingWith(second_prefix)
    if secondFullName.contains(sp.name)
    if fp.name.stripPrefix(first_prefix) == sp.name.stripPrefix(second_prefix)
    if !(excludes.contains(fp.name) || excludes.contains(sp.name))
  } yield ConnectedPortGroup(mi, fp, mloc.fullName, si, sp, direction)

  /**
    * Performs pre-connection checks for the port group connection (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

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
  override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
