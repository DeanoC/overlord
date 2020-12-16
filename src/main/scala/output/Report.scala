package output

import overlord.Connections.Connected
import overlord.Definitions._
import overlord.Instances.{BoardInstance, Container, Instance}

import java.nio.file.Path
import overlord._

import scala.collection.mutable
import ikuy_utils._

object Report {

	def apply(game: Game, out: Path): Unit = {

		val sb       = new StringBuilder
		val cpus     = game.cpus
		val cpuTypes = cpus.map(_.definition).toSet
		sb ++= (f"------------------%n")
		sb ++= (f"${cpus.length} CPU cores of ${cpuTypes.size} types%n")
		sb ++= (f"------------------%n")
		for (cput <- cpuTypes) {
			val chipType = cput.defType.ident.mkString
			val arch     = Utils.lookupString(cput.attributes, "arch", "UNKNOWN")
			val bw       = Utils.lookupInt(cput.attributes, "width", 32)

			sb ++= (f"${chipType} are ${bw} bit $arch CPUs%n")
			if (cput.software.nonEmpty)
				cput.software.get.groups.foreach(r => println(s"  ${r.description}"))
		}

		sb ++= f"%n------------------%n"
		sb ++= f"Instances%n"
		sb ++= f"------------------%n%n"

		for (cpu <- cpus) {
			sb ++= (f"cpu instance ${cpu.ident} of type " +
			        f"${cpu.definition.defType.ident.mkString(".")}%n")
		}

		sb ++= reportInstance(game)

		sb ++= (f"%n------------------%n")
		sb ++= (f"Connections%n")
		sb ++= (f"------------------%n%n")
		for (connection <- game.connections) {
			sb ++= (f"------------------%n")
			sb ++= f"${connection.firstFullName} <> ${connection.secondFullName}%n"
		}

		val setOfConnected = game.connections
			.filter(_.isConnected)
			.map(_.asConnected).toSet

		val setOfGateware = {
			val setOfGateware = mutable.HashSet[Instance]()
			setOfConnected.foreach { c =>
				if (c.first.nonEmpty && c.first.get.instance.isGateware)
					setOfGateware += c.first.get.instance
				if (c.second.nonEmpty && c.second.get.instance.isGateware)
					setOfGateware += c.second.get.instance
			}
			setOfGateware.toSet
		}

		val dm = game.distanceMatrix

		val boardIndex = dm.instanceArray.indexWhere(_.isInstanceOf[BoardInstance])

		sb ++= f"board has flat index $boardIndex%n"
		val connectionMask = Array.fill[Boolean](dm.dim)(elem = false)
		for {connected <- setOfConnected
		     (sp, ep) = dm.indicesOf(connected)} {
			connectionMask(sp) = true
			dm.routeBetween(sp, ep).foreach(connectionMask(_) = true)
		}
		dm.removeSelfLinks()
		dm.instanceMask(connectionMask)

		// solve for single links not passing through the board boundary
		val singleNonBoardLinks =
			for {connected <- setOfConnected
			     (sp, ep) = dm.indicesOf(connected)
			     if dm.distanceBetween(sp, ep) == 1
			     if sp != boardIndex && ep != boardIndex
			     } yield connected

		val setOfSNBL = mutable.HashMap[String,
			mutable.ArrayBuffer[(String, Connected)]]()

		for {connected <- singleNonBoardLinks}
			if (setOfSNBL.contains(connected.firstFullName))
				setOfSNBL(connected.firstFullName) +=
				((connected.secondFullName, connected))
			else if (setOfSNBL.contains(connected.secondFullName))
				setOfSNBL(connected.secondFullName) +=
				((connected.firstFullName, connected))
			else setOfSNBL += (connected.firstFullName ->
			                   mutable.ArrayBuffer(
				                   (connected.secondFullName, connected)))

		/*
				for {(_, sc) <- setOfSNBL
						 (_, connected) <- sc
						 sp = dm.instanceArray.indexOf(connected.first.get)
						 ep = dm.instanceArray.indexOf(connected.second.get)}
					sb ++= (f"route between $sp and $ep is ${dm.routeBetween(sp, ep)}" +
									f" of distance ${dm.distanceBetween(sp, ep)}%n")
		*/
		for {(wire, sc) <- setOfSNBL} {
			sb ++= f"chip to chip $wire to%n"
			for {(other, connected) <- sc} {
				assert(connected.isChipToChip)
				sb ++= f"\t$other%n"
			}
		}

		// solve for board connected pins
		val boardConnectedPins = mutable.HashMap[Int, Int]()
		for {connected <- setOfConnected
		     if connected.isPinToChip
		     first = connected.first.get
		     second = connected.second.get
		     (sp, ep) = dm.indicesOf(connected)
		     route = dm.routeBetween(sp, ep)
		     if route.contains(boardIndex)
		     } boardConnectedPins += (sp -> ep)

		boardConnectedPins.foreach(
			b => {
				val first  = dm.instanceArray(b._1)
				val second = dm.instanceArray(b._2)
				sb ++= f" ${first.ident} ${second.ident}%n"
			}
			)

		//		sb ++= dm.toString

		Utils.writeFile(Game.pathStack.top.resolve("report.txt"), sb.result())
	}

	private def reportInstance(container: Container,
	                           indentLevel: Int = 0): String = {
		val sb = new StringBuilder

		val indent = "\t" * indentLevel
		for (instance <- container.children) {
			sb ++= (indent + f"------------------%n")
			sb ++= (indent + instance.ident + f"%n")
			val id   = instance.definition.defType.ident.mkString(".")
			val name = (instance.definition.defType match {
				case RamDefinitionType(ident)      => "ram."
				case CpuDefinitionType(ident)      => "cpu."
				case NxMDefinitionType(ident)      => "NxM."
				case StorageDefinitionType(ident)  => "storage."
				case SocDefinitionType(ident)      => "soc."
				case BridgeDefinitionType(ident)   => "bridge."
				case NetDefinitionType(ident)      => "net."
				case OtherDefinitionType(ident)    => "other."
				case PinGroupDefinitionType(ident) => "pin."
				case ClockDefinitionType(ident)    => "clock."
				case ConstantDefinitionType(ident) => "constant."
				case BoardDefinitionType(ident)    => "board."
			}) + id
			sb ++= (indent + f"type: $name%n")
			instance match {
				case c: Container =>
					sb ++= reportInstance(c, indentLevel + 1)
				case _            =>
			}
		}

		sb.result()
	}
}
