package com.deanoc.overlord.connections

import com.deanoc.overlord.hardware.InWireDirection
import com.deanoc.overlord.instances.ChipInstance
import com.deanoc.overlord.DistanceMatrix
import com.deanoc.overlord.instances.{ClockInstance, PinGroupInstance}
import com.deanoc.overlord._
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
) {

  /** Checks if the starting location is a pin or clock.
    *
    * @return
    *   True if the starting location is a pin or clock, false otherwise.
    */
  def isStartPinOrClock: Boolean = startLoc.instance
    .isInstanceOf[PinGroupInstance] ||
    startLoc.instance
      .isInstanceOf[ClockInstance]

  /** Finds an ending location that is a pin or clock.
    *
    * @return
    *   An optional instance location that is a pin or clock.
    */
  def findEndIsPinOrClock: Option[InstanceLoc] =
    endLocs.find(il =>
      il.instance.isInstanceOf[PinGroupInstance] ||
        il.instance.isInstanceOf[ClockInstance]
    )
}

/** Companion object for managing wires.
  *
  * Provides methods for creating wires from logical connections and managing
  * intermediate ghost wires.
  */
object Wires {

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
  def apply(dm: DistanceMatrix, connected: Seq[Connected]): Seq[Wire] = {

    val wires = mutable.ArrayBuffer[Wire]()
    val ghosts = mutable.ArrayBuffer[GhostWire]()

    // connection are logical, wires are physical
    connected.foreach(c => {
      val (sp, ep) = c.direction match {
        case ConnectionDirection.FirstToSecond => dm.indicesOf(c)
        case ConnectionDirection.SecondToFirst =>
          (dm.indicesOf(c)._2, dm.indicesOf(c)._1)
        case ConnectionDirection.BiDirectional => dm.indicesOf(c)
      }
      if (!(sp < 0 || ep < 0 || c.first.isEmpty || c.second.isEmpty)) {
        val f = c.first.get
        val s = c.second.get
        val route = dm.routeBetween(sp, ep)

        var cp = sp
        for { p <- route } {
          val cploc =
            if (cp == sp) f
            else if (cp == ep) s
            else
              InstanceLoc(dm.instanceOf(cp), None, dm.instanceOf(cp).name)
          val ploc =
            if (p == sp) f
            else if (p == ep) s
            else
              InstanceLoc(dm.instanceOf(p), None, dm.instanceOf(p).name)

          ghosts += GhostWire(
            cp,
            p,
            cploc,
            ploc,
            c.direction,
            c.connectionPriority
          )
          cp = p
        }
      }
    })

    val (fanoutTmpWires, singleTmpWires) = {
      val fotWires =
        mutable.HashMap[
          InstanceLoc,
          (ConnectionPriority, mutable.ArrayBuffer[InstanceLoc])
        ]()
      val sTmpWires =
        mutable.HashMap[InstanceLoc, (ConnectionPriority, InstanceLoc)]()

      for {
        i <- ghosts.indices
        startLoc = ghosts(i).sloc
        endLoc = ghosts(i).eloc
        priority = ghosts(i).priority
      } {
        if (fotWires.contains(startLoc))
          fotWires(startLoc)._2 += endLoc
        else
          fotWires += (startLoc -> (priority, mutable.ArrayBuffer(endLoc)))
      }

      val multiFanoutTmpWires = (for ((sl, (pr, els)) <- fotWires) yield {
        if (els.length == 1) {
          if (
            sl.port.nonEmpty &&
            sl.port.get.direction == InWireDirection()
          )
            sTmpWires += ((els(0), ->(pr, sl)))
          else sTmpWires += (sl -> (pr, els(0)))
          None
        } else Some(sl -> (pr, els))
      }).flatten.toMap

      (multiFanoutTmpWires, sTmpWires)
    }

    wires ++= fanoutTmpWires.map { case (sl, (pr, els)) =>
      var knownWidth: Boolean =
        if (sl.port.isDefined) sl.port.get.knownWidth else true
      Wire(sl, els.toSeq, pr, knownWidth)
    }
    wires ++= singleTmpWires.map { case (sl, (pr, el)) =>
      val knownWidth: Boolean =
        if (sl.port.isDefined) sl.port.get.knownWidth else true
      Wire(sl, Seq(el), pr, knownWidth)
    }

    wires.sortInPlaceWith((a, b) =>
      a.startLoc.instance.name <
        b.startLoc.instance.name
    )

    wires.toSeq

  }
}
