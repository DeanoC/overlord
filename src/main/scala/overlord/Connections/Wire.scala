package overlord.Connections

import overlord.DistanceMatrix
import overlord.Gateware.{InOutWireDirection, InWireDirection, OutWireDirection}
import overlord.Instances.{ClockInstance, PinGroupInstance}

import scala.collection.mutable

case class Wire(startLoc: InstanceLoc,
                endLocs: Seq[InstanceLoc],
                priority: ConnectionPriority) {
	def isStartPinOrClock: Boolean = startLoc.instance
		                                 .isInstanceOf[PinGroupInstance] ||
	                                 startLoc.instance
		                                 .isInstanceOf[ClockInstance]

	def findEndIsPinOrClock: Option[InstanceLoc] =
		endLocs.find(il => il.instance.isInstanceOf[PinGroupInstance] ||
		                   il.instance.isInstanceOf[ClockInstance])
}

object Wires {

	private case class GhostWire(sp: Int,
	                             ep: Int,
	                             sloc: InstanceLoc,
	                             eloc: InstanceLoc,
	                             direction: ConnectionDirection,
	                             priority: ConnectionPriority)

	def apply(dm: DistanceMatrix,
	          connected: Seq[Connected]): Seq[Wire] = {

		val wires  = mutable.ArrayBuffer[Wire]()
		val ghosts = mutable.ArrayBuffer[GhostWire]()

		// connection are logical, wires are physical
		connected.foreach(
			c => {
				val (sp, ep) = c.direction match {
					case FirstToSecondConnection() => dm.indicesOf(c)
					case SecondToFirstConnection() => (dm.indicesOf(c)._2, dm.indicesOf(c)._1)
					case BiDirectionConnection()   => dm.indicesOf(c)
				}
				if (!(sp < 0 || ep < 0 || c.first.isEmpty || c.second.isEmpty)) {
					val f     = c.first.get
					val s     = c.second.get
					val route = dm.routeBetween(sp, ep)

					var cp = sp
					for {p <- route} {
						val cploc = if (cp == sp) f else if (cp == ep) s else
							InstanceLoc(dm.instanceOf(cp), None, dm.instanceOf(cp).ident)
						val ploc  = if (p == sp) f else if (p == ep) s else
							InstanceLoc(dm.instanceOf(p), None, dm.instanceOf(p).ident)

						ghosts += GhostWire(cp, p, cploc, ploc, c.direction, c.connectionPriority)
						cp = p
					}
				}
			})

		val (fanoutTmpWires, singleTmpWires) = {
			val fotWires  =
				mutable.HashMap[InstanceLoc,
					(ConnectionPriority, mutable.ArrayBuffer[InstanceLoc])]()
			val sTmpWires =
				mutable.HashMap[InstanceLoc, (ConnectionPriority, InstanceLoc)]()

			for {i <- ghosts.indices
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
					if (sl.port.nonEmpty &&
					    sl.port.get.direction == InWireDirection())
						sTmpWires += ((els(0), ->(pr, sl)))
					else sTmpWires += (sl -> (pr, els(0)))
					None
				} else Some(sl -> (pr, els))
			}).flatten.toMap

			(multiFanoutTmpWires, sTmpWires)
		}

		wires ++= fanoutTmpWires.map {
			case (sl, (pr, els)) => Wire(sl, els.toSeq, pr)
		}
		wires ++= singleTmpWires.map {
			case (sl, (pr, el)) => Wire(sl, Seq(el), pr)
		}

		wires.sortInPlaceWith((a, b) =>
			                      a.startLoc.instance.ident <
			                      b.startLoc.instance.ident)

		wires.toSeq

	}
}