package overlord

import overlord.Connections.{Connected, InstanceLoc}
import overlord.Instances.{BoardInstance, Container, Instance}

import scala.collection.mutable

case class DistanceMatrix(instanceArray: Array[Instance]) {
	val dim: Int = instanceArray.length

	private var virtualContainers = Seq[Instance]()

	private val distanceMatrix = Array.fill[Int](dim, dim) {
		Int.MaxValue
	}
	private val routeMatrix    = Array.ofDim[Seq[Int]](dim, dim)

	def +=(tuple:(Int, Int, (Int, Seq[Int]))): Unit = {
		val (s, e, v) = tuple
		distanceMatrix(s)(e) = v._1
		routeMatrix(s)(e) = v._2
	}

	def distanceBetween(s: Int, e: Int): Int = distanceMatrix(s)(e)

	def routeBetween(s: Int, e: Int): Seq[Int] = routeMatrix(s)(e)

	def between(s: Int, e: Int): (Int, Seq[Int]) =
		(distanceMatrix(s)(e), routeMatrix(s)(e))

	def rowDistance(r: Int): Seq[Int] = distanceMatrix(r).toSeq

	def columnDistance(c: Int): Seq[Int] =
		for (i <- 0 until dim) yield distanceMatrix(i)(c)

	def rowBetween(r: Int): Seq[Seq[Int]] = routeMatrix(r).toSeq

	def columnBetween(c: Int): Seq[Seq[Int]] =
		for (i <- 0 until dim) yield routeMatrix(i)(c)

	def instanceOf(i: Int): Instance = instanceArray(i)

	def indexOf(c: Instance): Int = instanceArray.indexOf(c)

	def indexOf(c: InstanceLoc): Int = indexOf(c.instance)

	def distanceBetween(a:Instance, b:Instance): Int =
		distanceBetween(indexOf(a), indexOf(b))

	def connected(a:Instance, b:Instance): Boolean = distanceBetween(a, b) > 0

	def indicesOf(c: Connected): (Int, Int) = (
		if (c.first.nonEmpty) indexOf(c.first.get) else -1,
		if (c.second.nonEmpty) indexOf(c.second.get) else -1)

	override def toString: String = {
		val sb = new StringBuilder
		sb ++= f"DistanceMatrix $dim x $dim%n"
		for {sp <- 0 until dim} {
			sb ++= "       " * sp
			for {ep <- sp until dim} {
				val e = if (distanceMatrix(sp)(ep) != Int.MaxValue)
					f"${distanceMatrix(sp)(ep)}%4d"
				else "    "
				sb ++= f"$e | "
			}
			sb ++= "\n"
		}
		sb.result()
	}

	private def removeSelfLinks(): Unit =
		for (i <- 0 until dim) distanceMatrix(i)(i) = Int.MaxValue

	private def instanceMask(maskArray: Array[Boolean]): Unit = {
		assert(maskArray.length == dim)

		for {i <- 0 until dim
		     if !maskArray(i)
		     j <- 0 until dim} {
			distanceMatrix(i)(j) = Int.MaxValue
			distanceMatrix(j)(i) = Int.MaxValue
		}
	}
}


object DistanceMatrix {

	private class Path(val sp: Int, val ep: Int) {
		var minDistance: Int      = Int.MaxValue
		var minRoute   : Seq[Int] = Seq()

		def setMin(dist: Int, route: Seq[Int]): Path = {
			minDistance = dist
			minRoute = route
			this
		}

		def setMin(route: (Int, Seq[Int])): Path = {
			minDistance = route._1
			minRoute = route._2
			this
		}

		override def toString: String =
			s"path between $sp, $ep, length $minDistance, route $minRoute"
	}

	private def flattenInstances(instances: Seq[Instance]): Array[Instance] = {
		(for (instance <- instances) yield
			instance match {
				case container: Container =>
					flattenInstances(container.children) ++
					(if (container.physical) Array(instance) else Array())
				case _                    => Array(instance)
			}
			).toArray.flatten
	}

	def apply(instances: Seq[Instance], connected: Seq[Connected]): DistanceMatrix = {
		val instancesArray = flattenInstances(instances)
		val dm             = DistanceMatrix(instancesArray)

		val containers = instancesArray.filter(_.isInstanceOf[Container])

		dm.virtualContainers = instances.filter {
			case container: Container => !container.physical
			case _                    => false
		}

		// instance to itself has a length of 0
		for {i <- 0 until dm.dim}
			dm += (i, i, (0, Seq(i)))

		// connected are 1 links
		for(con <- connected
		    if con.first.isDefined
		    if con.second.isDefined;
		    sinst = con.first.get.instance;
				einst = con.second.get.instance;
				si = instancesArray.indexOf(sinst);
				ei = instancesArray.indexOf(einst)
		    ) {
			dm += (si, ei, (1, Seq(ei)))
			dm += (ei, si, (1, Seq(si)))
		}

		// containers have a 1 link to there children
		for {containerInstance <- containers
		     container = containerInstance.asInstanceOf[Container]
		     if container.physical
		     sp = instancesArray.indexOf(containerInstance)
		     child <- container.children
		     ep = instancesArray.indexOf(child)} {
			dm += (sp, ep, (1, Seq(ep)))
			dm += (ep, sp, (1, Seq(sp)))
		}

		// top level to virtual container children
		for {containerInstance <- dm.virtualContainers
		     container = containerInstance.asInstanceOf[Container]
		     child <- container.children
		     inst <- instances
		     if containerInstance != inst
		     sp = instancesArray.indexOf(child)
		     ep = instancesArray.indexOf(inst)
		     } {
			dm += (sp, ep, (1, Seq(ep)))
			dm += (ep, sp, (1, Seq(sp)))
		}

		// compute the distance between instances
		for {sp <- 0 until dm.dim
		     ep <- 0 until dm.dim} {
			val path = new Path(sp, ep)
			computeDistanceBetween(dm, path.sp, path)
		}

		// remove all non connected left overs
		val connectionMask = Array.fill[Boolean](dm.dim)(elem = false)
		for {connected <- connected
		     (sp, ep) = dm.indicesOf(connected)} {
			connectionMask(sp) = true
			dm.routeBetween(sp, ep).foreach(connectionMask(_) = true)
		}
		dm.removeSelfLinks()
		dm.instanceMask(connectionMask)

		dm
	}

	private def computeDistanceBetween(dm: DistanceMatrix,
	                                   sp: Int,
	                                   path: Path): (Int, Seq[Int]) = {
		val ep = path.ep
		if (sp == ep) (0, Seq())
		else if (dm.distanceBetween(sp, ep) >= 0 &&
		         dm.distanceBetween(sp, ep) != Int.MaxValue) dm.between(sp, ep)
		else {
			// find possible segments in path
			val columnCandidates = dm.columnDistance(sp).zipWithIndex
				.filter(a => (a._1 >= 1 && a._1 != Int.MaxValue) && a._2 != sp)

			for (candidate <- columnCandidates) {
				val (ld, lr) = computeDistanceBetween(dm, candidate._2, path)
				val dist     = ld + candidate._1
				val route    = Seq(candidate._2) ++ lr

				// update matrix as we go
				if (dm.distanceBetween(sp, path.ep) >= dist) {
					dm += (sp, path.ep, (dist, route))
				}

				if (dist < path.minDistance) {
					path.setMin(dist, route)
					if (dist <= 1) return (dist, route)
				}
			}
			(path.minDistance, path.minRoute)
		}
	}
}