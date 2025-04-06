package overlord

import overlord.Connections._
import overlord.Instances.{ChipInstance, Container, PinGroupInstance}

import scala.collection.mutable
import scala.language.postfixOps
import scala.util.boundary, boundary.break

case class DistanceMatrix(instanceArray: Array[ChipInstance]) {
  val dim: Int = instanceArray.length
  private val distanceMatrix =
    Array.fill[Int](dim, dim)(DistanceMatrix.NotComputed)
  private val routeMatrix = Array.ofDim[Seq[Int]](dim, dim)
  private var virtualContainers = Seq[ChipInstance]()

  def routeBetween(s: Int, e: Int): Seq[Int] = routeMatrix(s)(e)

  def between(s: Int, e: Int): (Int, Seq[Int]) =
    (distanceMatrix(s)(e), routeMatrix(s)(e))

  def rowDistances(r: Int): Seq[Int] = distanceMatrix(r).toSeq

  def doesColumnHaveAnyLinks(c: Int): Boolean = validColumnDistances(c).nonEmpty

  def validColumnDistances(c: Int): Seq[Int] = columnDistances(c).filter(_ > 0)

  def columnDistances(c: Int): Seq[Int] = for (i <- 0 until dim)
    yield distanceMatrix(i)(c)

  def rowBetween(r: Int): Seq[Seq[Int]] = routeMatrix(r).toSeq

  def columnBetween(c: Int): Seq[Seq[Int]] = for (i <- 0 until dim)
    yield routeMatrix(i)(c)

  def instanceOf(i: Int): ChipInstance = instanceArray(i)

  def distanceBetween(s: Int, e: Int): Int = distanceMatrix(s)(e)

  // non index helpers of main functions
  def isConnectedBetween(a: ChipInstance, b: ChipInstance): Boolean =
    distanceBetween(a, b) > 0

  def distanceBetween(a: ChipInstance, b: ChipInstance): Int =
    distanceBetween(indexOf(a), indexOf(b))

  def indicesOf(c: Connected): (Int, Int) = (
    if (c.first.nonEmpty) indexOf(c.first.get) else -1,
    if (c.second.nonEmpty) indexOf(c.second.get) else -1
  )

  def connectedTo(c: ChipInstance): Seq[ChipInstance] =
    connectedTo(indexOf(c)).map(instanceOf(_))

  def indexOf(c: InstanceLoc): Int = indexOf(
    c.instance.asInstanceOf[ChipInstance]
  )

  def indexOf(c: ChipInstance): Int = instanceArray.indexOf(c)

  def connectedTo(srcIndex: Int): Seq[Int] = for {
    o <- 0 until dim
    if srcIndex != o
    d0 = distanceMatrix(srcIndex)(o)
    if !(d0 == DistanceMatrix.NotComputed)
  } yield o

  def expandedRouteBetween(
      s: ChipInstance,
      e: ChipInstance
  ): Seq[(ChipInstance, ChipInstance)] = {
    val route = routeBetween(s, e)
    Seq((instanceOf(indexOf(s)), instanceOf(route(0)))) ++ (for {
      i <- 1 until route.length
      si = route(i - 1)
      ei = route(i)
    } yield ((instanceOf(si), instanceOf(ei))))
  }

  def routeBetween(s: ChipInstance, e: ChipInstance): Seq[Int] =
    routeMatrix(indexOf(s))(indexOf(e))

  def debugPrint: String = {
    val sb = new StringBuilder

    // list instances that aren't isolated
    {
      sb ++= f"Instances count = ${instanceArray.length}%n"
      for {
        (instance, index) <- instanceArray.zipWithIndex
        if !isIsolated(index)
      } sb ++= f"$index - ${instance.name}%n"
      sb ++= f"---------------------------------%n"
    }

    // display compressed matrix (isolated nodes hidden)
    {
      sb ++= f"DistanceMatrix $dim x $dim start:horiz end:vertical%n"
      sb ++= f"     | "
      val ulsb = new StringBuilder()
      ulsb ++= "-------"
      for { i <- 0 until dim if !isIsolated(i) } {
        ulsb ++= "-------"
        sb ++= f"$i%4d | "
      }
      sb ++= "\n"
      sb ++= ulsb.toString()
      sb ++= "\n"

      for {
        sp <- 0 until dim
        if !isIsolated(sp)
      } {
        sb ++= f"$sp%4d | "
        for {
          ep <- 0 until dim
          if !isIsolated(ep)
        } {
          val e =
            if (distanceMatrix(sp)(ep) != DistanceMatrix.NotComputed)
              f"${distanceMatrix(sp)(ep)}%4d"
            else "    "
          sb ++= f"$e | "
        }
        sb ++= "\n"
      }
      sb ++= f"---------------------------------%n"
    }
    // display routes
    {
      sb ++= f"Routes%n"
      for {
        i <- 0 until dim
        if !isIsolated(i)
      } {
        for {
          j <- 0 until dim
          if !isIsolated(j)
        } {
          if (routeMatrix(i)(j) != null && routeMatrix(i)(j).nonEmpty) {
            sb ++= f"route between $i and $j, length ${routeMatrix(i)(j).length}%n"
            sb ++= f"$i"
            for { k <- routeMatrix(i)(j) } {
              sb ++= f" -> $k"
            }
            sb ++= f" ::  "
            sb ++= f"${instanceArray(i).name}"
            for { k <- routeMatrix(i)(j) } {
              sb ++= f" -> ${instanceArray(k).name}"
            }
            sb ++= f"%n"
          }
        }
      }
    }

    sb.result()
  }

  def isIsolated(i: Int): Boolean = {
    boundary {
      for {
        o <- 0 until dim
        if i != o
        d0 = distanceMatrix(i)(o)
        d1 = distanceMatrix(o)(i)
        if !(d0 == DistanceMatrix.NotComputed && d1 == DistanceMatrix.NotComputed)
      } {
        val minCount =
          if (
            instanceArray(i).isInstanceOf[PinGroupInstance] || instanceArray(o)
              .isInstanceOf[PinGroupInstance]
          ) 1
          else 0
        if (d0 > minCount || d1 > minCount) break(false)
      }
      true
    }
  }

}

object DistanceMatrix {
  private val NotComputed = -1

  def apply(
      instances: Seq[ChipInstance],
      connected: Seq[Connected]
  ): DistanceMatrix = {
    val instancesArray = flattenInstances(instances)
    val dm = DistanceMatrix(instancesArray)

    dm.virtualContainers = instances.filter {
      case container: Container => !container.physical
      case _                    => false
    }

    // compute the routes between instances
    computeRouteBetweens(dm, instancesArray.toIndexedSeq, connected)

    dm
  }

  private def flattenInstances(
      instances: Seq[ChipInstance]
  ): Array[ChipInstance] = {
    (for (instance <- instances) yield instance match {
      case container: Container =>
        flattenInstances(container.chipChildren) ++
          (if (container.physical) Array(instance) else Array[ChipInstance]())
      case _ => Array(instance)
    }).toArray.flatten
  }

  private def computeRouteBetweens(
      dm: DistanceMatrix,
      instancesArray: Seq[ChipInstance],
      connected: Seq[Connected]
  ): Unit = {
    val neighbours =
      Array.ofDim[mutable.ArrayBuffer[Int]](instancesArray.length)
    for { i <- 0 until dm.dim } neighbours(i) = mutable.ArrayBuffer[Int]()

    // connected are 1 links
    for (
      con <- connected
      if con.first.isDefined
      if con.second.isDefined;
      sinst = con.first.get.instance;
      einst = con.second.get.instance;
      si = instancesArray.indexOf(sinst);
      ei = instancesArray.indexOf(einst)
    ) {
      con.direction match {
        case FirstToSecondConnection() => neighbours(si) += ei
        case SecondToFirstConnection() => neighbours(ei) += si
        case BiDirectionConnection() =>
          neighbours(si) += ei
          neighbours(ei) += si
      }
    }
    // containers have a 1 link to there children
    for {
      containerInstance <- instancesArray.filter(_.isInstanceOf[Container])
      container = containerInstance.asInstanceOf[Container]
      if container.physical
      si = instancesArray.indexOf(containerInstance)
      child <- container.children
      ei = instancesArray.indexOf(child)
    } {
      neighbours(si) += ei
    }

    // top level to virtual container children
    for {
      containerInstance <- dm.virtualContainers
      container = containerInstance.asInstanceOf[Container]
      child <- container.children
      inst <- instancesArray
      if containerInstance != inst
      si = instancesArray.indexOf(child)
      ei = instancesArray.indexOf(inst)
    } {
      neighbours(si) += ei
    }

    val nodes = for { i <- 0 until dm.dim } yield Node(i, neighbours(i).toSeq)
    val dijkstraContext = Dijkstra()
    for { i <- 0 until dm.dim; j <- 0 until dm.dim } {
      dijkstraContext.solve(nodes, i, j) match {
        case Some((cost, path)) =>
          dm.distanceMatrix(i)(j) = cost
          dm.routeMatrix(i)(j) = path.map(_.index).tail
        case None => dm.routeMatrix(i)(j) = Seq()
      }
    }
  }

  case class Node(index: Int, neighbours: Seq[Int])

  case class Dijkstra() {
    type Cost = Int
    type Path = IndexedSeq[Node]
    type NextStep = (Cost, Node, Path)

    private implicit val nextStepSort: Ordering[(Cost, Node, Path)] =
      Ordering.fromLessThan[NextStep](_._1 < _._1).reverse

    def solve(
        nodes: Seq[Node],
        sourceIndex: Int,
        destIndex: Int
    ): Option[(Cost, IndexedSeq[Node])] = {
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
            val neighboursWithCost = currentNode.neighbours.map((_, 1))

            neighboursWithCost.foreach { case (neighbourIndex, neighbourCost) =>
              val neighbourNode = nodes(neighbourIndex)
              heap +=
                ((
                  currentCost + neighbourCost,
                  neighbourNode,
                  currentPath :+
                    neighbourNode
                ))
            }
          }
        }
      }
      None
    }
  }
}
