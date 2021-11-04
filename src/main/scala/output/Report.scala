package output

import ikuy_utils._
import overlord.Instances.{BoardInstance, ChipInstance, Container, InstanceTrait}
import overlord._

import java.nio.file.Path
import scala.collection.mutable

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
		}

		sb ++= f"%n------------------%n"
		sb ++= f"Instances%n"
		sb ++= f"------------------%n%n"

		sb ++= reportInstances(game)

		sb ++= f"%n------------------%n"
		sb ++= f"Connections%n"
		sb ++= f"------------------%n%n"
		for (connection <- game.connected) {
			sb ++= f"------------------%n"
			sb ++= f"${connection.firstFullName} <> ${connection.secondFullName}%n"
		}

		val boardIndex = game.distanceMatrix.instanceArray.indexWhere(_.isInstanceOf[BoardInstance])
		sb ++= f"%n------------------%n"
		sb ++= f"Board Index = $boardIndex%n"
		sb ++= f"------------------%n"

		// solve for board connected pins
		val boardConnectedPins = mutable.HashMap[Int, Int]()
		for {connected <- game.connected
		     if connected.isPinToChip
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

	private def reportInstances(game: Game): String = {
		game.children.map(reportInstance(_)).mkString("")
	}

	private def reportContainer(container: Container,
	                           indentLevel: Int = 0) : String = {
		container.children.map(reportInstance(_, indentLevel)).mkString("")
	}

	private def reportInstance(instance: InstanceTrait,
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
			case ProgramDefinitionType(ident)  => "program."
			case LibraryDefinitionType(ident)  => "library."
		}) + id
		sb ++= (indent + f"type: $name%n")
		instance match {
			case ci: ChipInstance =>
				for (rl <- ci.registerLists)
					sb ++= f"   ${rl.name} - ${rl.description}%n"
			case _                       =>
		}
		instance match {
			case c: Container =>
				sb ++= reportContainer(c, indentLevel + 1)
			case _            =>
			}

		sb.result()
	}
}
