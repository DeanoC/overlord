package overlord

import overlord.Connections.{Connected, InstanceLoc}
import overlord.Instances.{BoardInstance, Container, Instance}

import scala.collection.mutable

case class DistanceMatrix(instanceArray: Array[Instance]) {
	val dim: Int = instanceArray.length

	private var virtualContainers = Seq[Instance]()

	private val distanceMatrix = Array.fill[Int](dim, dim) { DistanceMatrix.NotComputed }
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

	def rowDistances(r: Int): Seq[Int] = distanceMatrix(r).toSeq

	def columnDistances(c: Int): Seq[Int] =
		for (i <- 0 until dim) yield distanceMatrix(i)(c)

	def validColumnDistances(c: Int): Seq[Int] = columnDistances(c).filter(_ > 0)

	def doesColumnHaveAnyLinks(c: Int): Boolean = validColumnDistances(c).nonEmpty

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

		sb ++= f"     | "
		for {i <- 0 until dim}
			sb ++= f"$i%4d | "
		sb ++= "\n"
		sb ++= "-------" * (dim+1)
		sb ++= "\n"

		for {sp <- 0 until dim} {
			sb ++= f"$sp%4d | "
			for {ep <- 0 until dim} {
				val e = if (distanceMatrix(sp)(ep) != DistanceMatrix.NotComputed)
					f"${distanceMatrix(sp)(ep)}%4d"
				else "    "
				sb ++= f"$e | "
			}
			sb ++= "\n"
		}
		for((instance, index) <- instanceArray.zipWithIndex)
			sb ++= f"$index - ${instance.ident}%n"

		sb.result()
	}

	private def setIdentityDiagonal(): Unit =
		for (i <- 0 until dim) distanceMatrix(i)(i) = 0

	private def instanceMask(maskArray: Array[Boolean]): Unit = {
		assert(maskArray.length == dim)

		for {i <- 0 until dim
		     if !maskArray(i)
		     j <- 0 until dim} {
			distanceMatrix(i)(j) = DistanceMatrix.NoRoute
			distanceMatrix(j)(i) = DistanceMatrix.NoRoute
		}
	}
}


object DistanceMatrix {
	private val NotComputed = -1
	private val NoRoute = -2

	private case class Path(val sp: Int, val ep: Int) {
		var minDistance: Int      = DistanceMatrix.NotComputed
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
		dm.setIdentityDiagonal()

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

		// any columns with not links can't be linked to anything
		for { sp <- 0 until dm.dim } {
			if(!dm.doesColumnHaveAnyLinks(sp)) {
				for(i <- 0 until dm.dim
				    if sp != i) {
					dm.distanceMatrix(i)(sp) = DistanceMatrix.NoRoute
					dm.distanceMatrix(sp)(i) = DistanceMatrix.NoRoute
				}
			}
		}

		// compute the distance between instances
		for {sp <- 0 until dm.dim
		     ep <- sp until dm.dim
		     if sp != ep } {
				computeDistanceBetween(dm, sp, Path(sp, ep))
		}

		dm.setIdentityDiagonal()

		dm
	}

	private def computeDistanceBetween(dm: DistanceMatrix,
	                                   sp: Int,
	                                   path: Path,
	                                   tried: Seq[Int] = Seq()) : (Int, Seq[Int]) = {
		val ep = path.ep
		if (sp == ep) (0, Seq())
		else if(tried.contains(sp)) (NoRoute, Seq())
		else if(dm.distanceMatrix(sp)(ep) == NoRoute) (NoRoute, Seq())
		else if (dm.distanceBetween(sp, ep) >= 0) dm.between(sp, ep)
		else if (dm.distanceBetween(ep, sp) >= 0) dm.between(sp, ep)
		else {
			// find possible segments in path
			val col = dm.columnDistances(sp).zipWithIndex
			val columnCandidates = col.filter(_._1 >= 1)
				.filter(_._2 != sp)
				.filter(p => !tried.contains(p._2))
			if(columnCandidates.isEmpty) return (NoRoute, Seq())

			for (candidate <- columnCandidates) {
				val (ld, lr) = computeDistanceBetween(dm, candidate._2, path, Seq(sp) ++ tried)
				if(ld != NotComputed && ld != NoRoute) {
					val dist  = ld + candidate._1
					val route = Seq(candidate._2) ++ lr

					// update matrix as we go
					val db = dm.distanceBetween(sp, path.ep)
					if (db < 0 || db >= dist) {
						dm += (sp, path.ep, (dist, route))
						dm += (path.ep, sp, (dist, route.reverse))
					}

					if (dist < path.minDistance) {
						path.setMin(dist, route)
						if (dist <= 1) return (dist, route)
					}
				}
			}
			(path.minDistance, path.minRoute)
		}
	}
}