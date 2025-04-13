package com.deanoc.overlord.connections

import com.deanoc.overlord._
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.interfaces.PortsLike

/** Represents an unconnected port connection between two components.
  *
  * This case class defines the properties and methods for managing unconnected
  * port connections, including establishing connections and collecting
  * constants.
  *
  * @param firstFullName
  *   The full name of the first component in the connection.
  * @param direction
  *   The direction of the connection.
  * @param secondFullName
  *   The full name of the second component in the connection.
  */
case class UnconnectedPort(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String
) extends Unconnected {

  /** Establishes the port connection between components.
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
    fp <- mi.getPortsStartingWith("")
    if firstFullName.split('.').map(_ == fp.name).reduce((a, b) => a | b)
    sp <- si.getPortsStartingWith("")
    if secondFullName.split('.').map(_ == sp.name).reduce((a, b) => a | b)
  } yield ConnectedPortGroup(mi, fp, mloc.fullName, si, sp, direction)

  /** Performs pre-connection checks for the port connection (no operation for
    * this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

  /** Collects constants associated with the unconnected port.
    *
    * @param unexpanded
    *   A sequence of unexpanded instance traits.
    * @return
    *   A sequence of constants (empty for this implementation).
    */
  override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] =
    Seq()

  /** Finalizes the port connection (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
