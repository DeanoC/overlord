package com.deanoc.overlord.connections

import com.deanoc.overlord.config.WireDirection
import com.deanoc.overlord.instances.HardwareInstance
import com.deanoc.overlord.DistanceMatrix
import com.deanoc.overlord.instances.{ClockInstance, PinGroupInstance}
import com.deanoc.overlord._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.connections.ConnectionTypes.{InstanceName}
import scala.collection.mutable

/** Represents a physical wire connection between components.
  *
  * @param startLoc
  *   The starting location of the wire.
  * @param endLocs
  *   A sequence of ending locations for the wire.
  * @param priority
  *   The priority of the connection.
  * @param knownWidth
  *   Indicates whether the width of the wire is known.
  */
case class Wire(
    startLoc: InstanceLoc,
    endLocs: Seq[InstanceLoc],
    priority: ConnectionPriority,
    knownWidth: Boolean
):
  /** Checks if the starting location is a pin or clock.
    *
    * @return
    *   True if the starting location is a pin or clock, false otherwise.
    */
  def isStartPinOrClock: Boolean = startLoc.instance match
    case _: PinGroupInstance | _: ClockInstance => true
    case _                                      => false

  /** Finds an ending location that is a pin or clock.
    *
    * @return
    *   An optional instance location that is a pin or clock.
    */
  def findEndIsPinOrClock: Option[InstanceLoc] =
    endLocs.find: il =>
      il.instance match
        case _: PinGroupInstance | _: ClockInstance => true
        case _                                      => false

/** Companion object for managing wires.
  *
  * Provides methods for creating wires from logical connections and managing
  * intermediate ghost wires.
  */
object Wires:
  /** Represents an intermediate ghost wire used during wire creation.
    *
    * @param sp
    *   The starting point index in the distance matrix.
    * @param ep
    *   The ending point index in the distance matrix.
    * @param sloc
    *   The starting location of the ghost wire.
    * @param eloc
    *   The ending location of the ghost wire.
    * @param direction
    *   The direction of the connection.
    * @param priority
    *   The priority of the connection.
    */
  private case class GhostWire(
      sp: Int,
      ep: Int,
      sloc: InstanceLoc,
      eloc: InstanceLoc,
      direction: ConnectionDirection,
      priority: ConnectionPriority
  )

  /** Creates a sequence of physical wires from logical connections.
    *
    * @param dm
    *   The distance matrix used to determine routing.
    * @param connected
    *   A sequence of logical connections.
    * @return
    *   A sequence of physical wires.
    */
  def apply(dm: DistanceMatrix, connected: Seq[Connected]): Seq[Wire] =
    val wires = mutable.ArrayBuffer.empty[Wire]
    val ghosts = mutable.ArrayBuffer.empty[GhostWire]

    // connection are logical, wires are physical
    for c <- connected do
      val (sp, ep) = c.direction match
        case ConnectionDirection.FirstToSecond => dm.indicesOf(c)
        case ConnectionDirection.SecondToFirst =>
          (dm.indicesOf(c)._2, dm.indicesOf(c)._1)
        case ConnectionDirection.BiDirectional => dm.indicesOf(c)

      // Use Option pattern matching with guard
      if (sp >= 0 && ep >= 0) then
        (c.first, c.second) match
          case (Some(f), Some(s)) =>
            val route = dm.routeBetween(sp, ep)

            var cp = sp
            for p <- route do
              // Create cploc based on position
              val cploc =
                if cp == sp then f
                else if cp == ep then s
                else
                  InstanceLoc(dm.instanceOf(cp), None, dm.instanceOf(cp).name)

              // Create ploc based on position
              val ploc =
                if p == sp then f
                else if p == ep then s
                else InstanceLoc(dm.instanceOf(p), None, dm.instanceOf(p).name)

              ghosts += GhostWire(
                cp,
                p,
                cploc,
                ploc,
                c.direction,
                c.connectionPriority
              )
              cp = p
          case _ => () // First or second is None
      else () // Invalid indices

    val (fanoutTmpWires, singleTmpWires) =
      val fotWires =
        mutable.HashMap.empty[
          InstanceLoc,
          (ConnectionPriority, mutable.ArrayBuffer[InstanceLoc])
        ]
      val sTmpWires =
        mutable.HashMap.empty[InstanceLoc, (ConnectionPriority, InstanceLoc)]

      for
        i <- ghosts.indices
        startLoc = ghosts(i).sloc
        endLoc = ghosts(i).eloc
        priority = ghosts(i).priority
      do
        if fotWires.contains(startLoc) then fotWires(startLoc)._2 += endLoc
        else fotWires += (startLoc -> (priority, mutable.ArrayBuffer(endLoc)))

      val multiFanoutTmpWires =
        for (sl, (pr, els)) <- fotWires
        yield els match
          case arr
              if arr.length == 1 && sl.port.exists(
                _.direction == WireDirection.Input
              ) =>
            sTmpWires += (arr(0) -> (pr, sl))
            None
          case arr if arr.length == 1 =>
            sTmpWires += (sl -> (pr, arr(0)))
            None
          case _ =>
            Some(sl -> (pr, els.toSeq))

      (multiFanoutTmpWires.flatten.toMap, sTmpWires.toMap)

    // Process fanout wires using concise function syntax
    wires ++= fanoutTmpWires.map:
      case (sl, (pr, els)) =>
        Wire(sl, els, pr, sl.port.map(_.knownWidth).getOrElse(true))

    // Process single wires using concise function syntax
    wires ++= singleTmpWires.map:
      case (sl, (pr, el)) =>
        Wire(sl, Seq(el), pr, sl.port.map(_.knownWidth).getOrElse(true))

    // Sort the wires by instance name using concise function syntax
    wires.sortInPlaceWith:
      case (a, b) =>
        val nameA = InstanceName(a.startLoc.instance.name)
        val nameB = InstanceName(b.startLoc.instance.name)
        nameA < nameB

    wires.toSeq
