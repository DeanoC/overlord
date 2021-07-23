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
		sb ++= f"------------------%n"
		sb ++= f"${cpus.map(_.cpuCount).sum} CPU cores of ${cpuTypes.size} different types%n"
		sb ++= f"------------------%n"
		for (cput <- cpuTypes) {
			val chipType = cput.defType.ident.mkString
			val arch     = Utils.lookupString(cput.attributes, "arch", "UNKNOWN")
			val bw       = Utils.lookupInt(cput.attributes, "width", 32)

			val coreCount = cpus.filter(_.definition == cput).map(_.cpuCount).sum

			sb ++= f"$coreCount cores of $chipType ($bw bit $arch) CPU%n"

			for(rl <- cput.registerLists)
				sb ++= f"   ${rl.name} - ${rl.description}%n"
		}

		sb ++= f"%n------------------%n"
		sb ++= f"Instances%n"
		sb ++= f"------------------%n%n"

		sb ++= reportGame(game)

		sb ++= f"%n------------------%n"
		sb ++= f"Connections%n"
		sb ++= f"------------------%n%n"
		for (connection <- game.connected) {
			sb ++= f"------------------%n"
			sb ++= f"${connection.firstFullName} <> ${connection.secondFullName}%n"
		}

		val boardIndex = game.distanceMatrix.instanceArray.indexWhere(_.isInstanceOf[BoardInstance])

		// solve for single links not passing through the board boundary
		val singleNonBoardLinks =
			for {connected <- game.connected
			     (sp, ep) = game.distanceMatrix.indicesOf(connected)
			     if game.distanceMatrix.distanceBetween(sp, ep) == 1
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
//				assert(connected.isChipToChip)
				sb ++= f"\t$other%n"
			}
		}

		// solve for board connected pins
		val boardConnectedPins = mutable.HashMap[Int, Int]()
		for {connected <- game.connected
		     if connected.isPinToChip
		     first = connected.first.get
		     second = connected.second.get
		     (sp, ep) = game.distanceMatrix.indicesOf(connected)
		     route = game.distanceMatrix.routeBetween(sp, ep)
		     if route.contains(boardIndex)
		     } boardConnectedPins += (sp -> ep)

		boardConnectedPins.foreach(
			b => {
				val first  = game.distanceMatrix.instanceArray(b._1)
				val second = game.distanceMatrix.instanceArray(b._2)
				sb ++= f" ${first.ident} ${second.ident}%n"
			}
			)

		sb ++= game.distanceMatrix.toString

		Utils.writeFile(Game.pathStack.top.resolve("report.txt"), sb.result())
	}

	private def reportGame(game: Game): String = {
		game.children.map(reportInstance(_)).mkString("")
	}

	private def reportContainer(container: Container,
	                           indentLevel: Int = 0) : String = {
		container.children.map(reportInstance(_, indentLevel)).mkString("")
	}

	private def reportInstance(instance: Instance,
	                           indentLevel: Int = 0): String = {
		val sb = new StringBuilder

		val indent = "\t" * indentLevel
		sb ++= (indent + f"------------------%n")
		sb ++= (indent + instance.ident + f"%n")
		val id   = instance.definition.defType.ident.mkString(".")
		val name = (instance.definition.defType match {
			case RamDefinitionType(ident)      => "ram."
			case CpuDefinitionType(ident)      => "cpu."
			case BusDefinitionType(ident)      => "bus."
			case StorageDefinitionType(ident)  => "storage."
			case BridgeDefinitionType(ident)   => "bridge."
			case NetDefinitionType(ident)      => "net."
			case OtherDefinitionType(ident)    => "other."
			case PinGroupDefinitionType(ident) => "pin."
			case ClockDefinitionType(ident)    => "clock."
			case BoardDefinitionType(ident)    => "board."
		}) + id
		sb ++= (indent + f"type: $name%n")
		instance match {
			case c: Container =>
				sb ++= reportContainer(c, indentLevel + 1)
			case _            =>
			}

		sb.result()
	}
}
