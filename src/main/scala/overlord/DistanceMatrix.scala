package overlord

import overlord.Connections.{BiDirectionConnection, Connected, FirstToSecondConnection, InstanceLoc, SecondToFirstConnection}
import overlord.Instances.{Container, Instance}

import scala.collection.mutable

case class DistanceMatrix(instanceArray: Array[Instance]) {
	val dim: Int = instanceArray.length

	private var virtualContainers = Seq[Instance]()

	private val distanceMatrix = Array.fill[Int](dim, dim) { DistanceMatrix.NotComputed }
	private val routeMatrix    = Array.ofDim[Seq[Int]](dim, dim)

	def distanceBetween(s: Int, e: Int): Int = {
		distanceMatrix(s)(e)
	}

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

		sb ++= f"Instances count = ${instanceArray.length}%n"
		for((instance, index) <- instanceArray.zipWithIndex)
			sb ++= f"$index - ${instance.ident}%n"
		sb ++= f"---------------------------------%n"

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

		sb ++= f"---------------------------------%n"
		sb ++= f"Routes%n"
		for {i <- 0 until dim} {
			for {j <- 0 until dim} {
				if(routeMatrix(i)(j) != null) {
					sb ++= f"route between $i and $j, length ${routeMatrix(i)(j).length}%n"
					sb ++= f"$i"
					for {k <- routeMatrix(i)(j)} {
						sb ++= f" -> $k"
					}
					sb ++= f" ::  "
					sb ++= f"${instanceArray(i).ident}"
					for {k <- routeMatrix(i)(j)} {
						sb ++= f" -> ${instanceArray(k).ident}"
					}
					sb ++= f"%n"
				}
			}
		}

		sb.result()
	}
}


object DistanceMatrix {
	private val NotComputed = -1

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

		dm.virtualContainers = instances.filter {
			case container: Container => !container.physical
			case _                    => false
		}

		// compute the routes between instances
		computeRouteBetweens(dm,instancesArray, connected)

		dm
	}

	case class Node(index: Int, neighbours: Seq[Int])

	case class Dijkstra() {
		type Cost = Int
		type Path = IndexedSeq[Node]
		type NextStep = (Cost, Node, Path)

		private implicit val nextStepSort: Ordering[(Cost, Node, Path)] =
			Ordering.fromLessThan[NextStep](_._1 < _._1).reverse


		def solve(nodes: Seq[Node], sourceIndex: Int, destIndex: Int): Option[(Cost,IndexedSeq[Node] )] = {
			val source = nodes(sourceIndex)
			val dest = nodes(destIndex)

			var visited = Set.empty[Node]
			val heap = new mutable.PriorityQueue[NextStep]
			heap += ((0, source, IndexedSeq(source)))

			while (heap.nonEmpty) {
				val (currentCost, currentNode, currentPath) = heap.dequeue()
				if (!visited.contains(currentNode)) {
					visited += currentNode
					if (currentNode == dest) {
						return Some(currentCost, currentPath)
					} else {
						val neighboursWithCost = currentNode.neighbours.map((_,1))

						neighboursWithCost.foreach { case (neighbourIndex, neighbourCost) =>
							val neighbourNode = nodes(neighbourIndex)
							heap += ((currentCost + neighbourCost, neighbourNode, currentPath :+ neighbourNode))
						}
					}
				}
			}
			None
		}
	}
	private def computeRouteBetweens(dm: DistanceMatrix,
	                                 instancesArray: Seq[Instance],
	                                 connected: Seq[Connected]): Unit = {
		val neighbours = Array.ofDim[mutable.ArrayBuffer[Int]](instancesArray.length)
		for{i <- 0 until dm.dim} neighbours(i) = mutable.ArrayBuffer[Int]()

		// connected are 1 links
		for (con <- connected
		     if con.first.isDefined
		     if con.second.isDefined;
		     sinst = con.first.get.instance;
		     einst = con.second.get.instance;
		     si = instancesArray.indexOf(sinst);
		     ei = instancesArray.indexOf(einst)
		     ) {
			con.direction match {
				case FirstToSecondConnection() => neighbours(si) += ei
				case SecondToFirstConnection() =>	neighbours(ei) += si
				case BiDirectionConnection()   =>
					neighbours(si) += ei
					neighbours(ei) += si
			}
		}
		// containers have a 1 link to there children
		for {containerInstance <- instancesArray.filter(_.isInstanceOf[Container])
		     container = containerInstance.asInstanceOf[Container]
		     if container.physical
		     si = instancesArray.indexOf(containerInstance)
		     child <- container.children
		     ei = instancesArray.indexOf(child)} {
			neighbours(si) += ei
		}

		// top level to virtual container children
		for {containerInstance <- dm.virtualContainers
		     container = containerInstance.asInstanceOf[Container]
		     child <- container.children
		     inst <- instancesArray
		     if containerInstance != inst
		     si = instancesArray.indexOf(child)
		     ei = instancesArray.indexOf(inst)
		     } {
			neighbours(si) += ei
		}


		val nodes = for{i <- 0 until dm.dim} yield Node(i, neighbours(i).toSeq)
		val dijkstraContext = Dijkstra()
		for{i <- 0 until dm.dim; j <- 0 until dm.dim} {
			dijkstraContext.solve(nodes, i, j) match {
				case Some((cost, path)) =>
					dm.distanceMatrix(i)(j) = cost
					dm.routeMatrix(i)(j) = path.map(_.index).tail
				case None               =>
			}
		}
	}
}