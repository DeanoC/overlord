package com.deanoc.overlord.connections

import com.deanoc.overlord._

import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}

/** Represents an unconnected clock connection between two components.
  *
  * This case class defines the properties and methods for managing unconnected
  * clock connections, including establishing connections and collecting
  * constants.
  *
  * @param firstFullName
  *   The full name of the first component in the connection.
  * @param direction
  *   The direction of the connection.
  * @param secondFullName
  *   The full name of the second component in the connection.
  */
case class UnconnectedClock(
    firstFullName: String,
    direction: ConnectionDirection,
    secondFullName: String
) extends Unconnected {

  /** Establishes the connection between the clock components.
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    * @return
    *   A sequence of connected components.
    */
  override def connect(unexpanded: Seq[ChipInstance]): Seq[Connected] = {
    val mo = matchInstances(firstFullName, unexpanded)
    val so = matchInstances(secondFullName, unexpanded)

    val cbp =
      if (mo.length > 1 || so.length > 1)
        WildCardConnectionPriority()
      else ExplicitConnectionPriority()

    for { mloc <- mo; sloc <- so } yield ConnectPortBetween(cbp, mloc, sloc)
  }

  /** Performs pre-connection checks for the clock connection.
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def preConnect(unexpanded: Seq[ChipInstance]): Unit = None

  /** Collects constants associated with the unconnected clock.
    *
    * @param unexpanded
    *   A sequence of unexpanded instance traits.
    * @return
    *   A sequence of constants (empty for this implementation).
    */
  override def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant] =
    Seq()

  /** Finalizes the clock connection (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit = None
}
