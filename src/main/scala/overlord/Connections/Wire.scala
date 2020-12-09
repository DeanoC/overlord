package overlord.Connections

import overlord.DistanceMatrix
import overlord.Gateware.{InOutWireDirection, InWireDirection, OutWireDirection}

import scala.collection.mutable

case class Wire(startLoc: InstanceLoc,
                endLocs: Seq[InstanceLoc])

object Wires {

	private case class GhostWire(sp: Int,
	                             ep: Int,
	                             sloc: InstanceLoc,
	                             eloc: InstanceLoc,
	                             direction: ConnectionDirection)

	def apply(dm: DistanceMatrix,
	          connected: Seq[Connected]): Seq[Wire] = {

		val wires  = mutable.ArrayBuffer[Wire]()
		val ghosts = mutable.ArrayBuffer[GhostWire]()

		// connection are logical, wires are physical
		connected.foreach(
			c => {
				val (sp, ep) = dm.indicesOf(c)

				if (!(sp < 0 || ep < 0 ||
				      c.first.isEmpty || c.second.isEmpty)) {
					val f     = c.first.get
					val s     = c.second.get
					val route = dm.routeBetween(sp, ep)

					var cp = sp
					for {p <- route} {
						val cploc = if (cp == sp) f else if (cp == ep) s else
							InstanceLoc(dm.instanceOf(cp),
							            None,
							            dm.instanceOf(cp).ident)
						val ploc  = if (p == sp) f else if (p == ep) s else
							InstanceLoc(dm.instanceOf(p),
							            None,
							            dm.instanceOf(p).ident)

						// validate directions
						if (cploc.port.nonEmpty) {
							cploc.port.get.direction match {
								case InWireDirection()  =>
									if (c.direction != SecondToFirstConnection() &&
									    c.direction != BiDirectionConnection()) {
										println(s"${cploc.fullName} is an input wire isn't " +
										        s"connected like that")
									}
								case OutWireDirection() =>
									if (c.direction != FirstToSecondConnection() &&
									    c.direction != BiDirectionConnection()) {
										println(s"${cploc.fullName} is an output wire isn't " +
										        s"connected like that")
									}
								case _                  =>
							}
						}
						if (ploc.port.nonEmpty) {
							ploc.port.get.direction match {
								case OutWireDirection() =>
									if (c.direction != SecondToFirstConnection() &&
									    c.direction != BiDirectionConnection()) {
										println(s"${ploc.fullName} is an output wire isn't " +
										        s"connected like that")
									}
								case InWireDirection()  =>
									if (c.direction != FirstToSecondConnection() &&
									    c.direction != BiDirectionConnection()) {
										println(s"${ploc.fullName} is an input wire isn't " +
										        s"connected like that")
									}
								case _                  =>
							}
						}
						if (c.direction == SecondToFirstConnection())
							ghosts += GhostWire(p, cp, ploc, cploc, c.direction.flip)
						else if (c.direction == BiDirectionConnection() && cp > p)
							ghosts += GhostWire(p, cp, ploc, cploc, c.direction)
						else
							ghosts += GhostWire(cp, p, cploc, ploc, c.direction)
						cp = p
					}
				}
			})

		val (fanoutTmpWires, singleTmpWires) = {
			val fotWires =
				mutable.HashMap[InstanceLoc, mutable.ArrayBuffer[InstanceLoc]]()

			for {i <- ghosts.indices
			     startLoc = ghosts(i).sloc
			     endLoc = ghosts(i).eloc
			     } {
				if (fotWires.contains(startLoc))
					fotWires(startLoc) += endLoc
				else
					fotWires += (startLoc -> mutable.ArrayBuffer(endLoc))
			}

			val sTmpWires = mutable.HashMap[InstanceLoc, InstanceLoc]()

			val multiFanoutTmpWires = (for (fo <- fotWires) yield {
				if (fo._2.length == 1) {
					if (fo._1.port.nonEmpty &&
					    fo._1.port.get.direction == InWireDirection())
						sTmpWires += (fo._2(0) -> fo._1)
					else sTmpWires += (fo._1 -> fo._2(0))
					None
				} else Some(fo)
			}).flatten.toMap


			(multiFanoutTmpWires, sTmpWires)
		}

		wires ++= fanoutTmpWires.map(w => Wire(w._1, w._2.toSeq))
		wires ++= singleTmpWires.map(w => Wire(w._1, Seq(w._2)))

		wires.toSeq
	}
}