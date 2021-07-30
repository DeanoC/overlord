package overlord

import ikuy_utils._
import overlord.Chip.RegisterBank
import overlord.Connections._
import overlord.Instances._

import java.nio.file.Path
import scala.collection.mutable

case class Game(name: String,
                children: Seq[ChipInstance],
                connected: Seq[Connected],
                distanceMatrix: DistanceMatrix,
                wires: Seq[Wire]
               ) {
	lazy val setOfGateware: Set[ChipInstance] = {
		val setOfGateware = mutable.HashSet[ChipInstance]()
		connected.foreach { c =>
			if (c.first.nonEmpty && c.first.get.isGateware)
				setOfGateware += c.first.get.instance
			if (c.second.nonEmpty && c.second.get.isGateware)
				setOfGateware += c.second.get.instance
		}
		setOfGateware.toSet
	}

	lazy val flatChipChildren: Seq[ChipInstance] =
		children.filter(_.isInstanceOf[Container])
			.map(_.asInstanceOf[Container])
			.flatMap(_.flatChildren.map(_.asInstanceOf[ChipInstance])) ++ children

	lazy val allChipInstances: Seq[ChipInstance] = flatChipChildren

	lazy val cpus: Seq[CpuInstance] = flatChipChildren
		.filter(_.isInstanceOf[CpuInstance])
		.map(_.asInstanceOf[CpuInstance])

	lazy val rams: Seq[RamInstance] =
		flatChipChildren.filter(_.isInstanceOf[RamInstance])
			.map(_.asInstanceOf[RamInstance])

	lazy val buses: Seq[BusInstance] =
		flatChipChildren.filter(_.isInstanceOf[BusInstance])
			.map(_.asInstanceOf[BusInstance])

	lazy val storages: Seq[StorageInstance] =
		flatChipChildren.filter(_.isInstanceOf[StorageInstance])
			.map(_.asInstanceOf[StorageInstance])

	lazy val nets: Seq[NetInstance] =
		flatChipChildren.filter(_.isInstanceOf[NetInstance])
			.map(_.asInstanceOf[NetInstance])

	lazy val pins: Seq[PinGroupInstance] =
		flatChipChildren.filter(_.isInstanceOf[PinGroupInstance])
			.map(_.asInstanceOf[PinGroupInstance])

	lazy val clocks: Seq[ClockInstance] =
		flatChipChildren.filter(_.isInstanceOf[ClockInstance])
			.map(_.asInstanceOf[ClockInstance])

	lazy val gatewares: Seq[ChipInstance] =
		flatChipChildren.filter(_.definition.isInstanceOf[GatewareDefinitionTrait])

	lazy val constants: Seq[ConnectedConstant] =
		connected.filter(_.isInstanceOf[ConnectedConstant])
			.map(_.asInstanceOf[ConnectedConstant])

	lazy val board: Option[BoardInstance] =
		children.find(_.isInstanceOf[BoardInstance])
			.asInstanceOf[Option[BoardInstance]]

	def getDirectBusesConnectedTo(instance: ChipInstance): Seq[BusInstance] =
		buses.filter(distanceMatrix.distanceBetween(instance, _) == 1)

	def getEndBusesConnecting(start: ChipInstance, end: ChipInstance): Seq[BusInstance] = {
		val startBuses = getBusesConnectedTo(start)
		(for (bus <- startBuses) yield {
			val connected = distanceMatrix.connected(bus, end)
			if (connected) Some(bus) else None
		}).flatten
	}

	def getBusesConnectedTo(instance: ChipInstance): Seq[BusInstance] =
		buses.filter(distanceMatrix.connected(instance, _))

	def getRAMConnectedTo(instance: ChipInstance): Seq[RamInstance] =
		rams.filter(distanceMatrix.connected(instance, _))

}

object Game {
	// these are mutable for easy backup and restore
	var pathStack     : mutable.Stack[Path]         = mutable.Stack()
	var containerStack: mutable.Stack[MutContainer] = mutable.Stack()

	def apply(gameName: String,
	          gamePath: Path,
	          out: Path,
	          catalogs: DefinitionCatalog): Option[Game] = {
		val gb = new GameBuilder(gameName, gamePath, catalogs)
		gb.toGame(out)
	}

	private class GameBuilder(gameName: String,
	                          gamePath: Path,
	                          catalogs: DefinitionCatalog) {

		private val defaults = mutable.Map[String, Variant]()


		process(gamePath, catalogs)

		def toGame(out: Path,
		           doPhase1: Boolean = true,
		           doPhase2: Boolean = true): Option[Game] = {
			val softPath = out.resolve("soft")
			val gatePath = out.resolve("gate")
			Utils.ensureDirectories(softPath)
			Utils.ensureDirectories(gatePath)

			val top = containerStack.head
			for (c <- containerStack.tail) {
				top.children ++= c.children
				top.connections ++= c.connections
			}
			containerStack.clear()

			val chipInstances = top.children.
				filter(_.isInstanceOf[ChipInstance]).
				map(_.asInstanceOf[ChipInstance]).toSeq

			Connection.preConnect(top.connections.toSeq, chipInstances)

			OutputGateware(top, gatePath, 1)
			OutputGateware(top, gatePath, 2)

			val connected = Connection.connect(top.connections.toSeq, chipInstances)

			// instances that are connected to buses need a register bank
			val buses = connected.filter(_.isInstanceOf[BusInstance])
				.map(_.asInstanceOf[BusInstance])
			for (bus <- buses;
			     inst <- bus.consumerInstances;
			     rl <- inst.instanceRegisterLists
			     if !inst.instanceRegisterBanks.exists(_.name == rl.name)) {

				val (address, _) = bus.getFirstConsumerAddressAndSize(inst)
				inst.instanceRegisterBanks +=
				RegisterBank(s"${bus.ident}_${inst.ident}", address, rl.name)
			}

			val setOfConnected = connected
				.filter(_.isConnected)
				.map(_.asConnected).toSet

			val dm: DistanceMatrix = DistanceMatrix(chipInstances, setOfConnected.toSeq)
			val wires              = Wires(dm, setOfConnected.toSeq)

			Some(Game(gameName, chipInstances, setOfConnected.toSeq, dm, wires))
		}

		private def process(path: Path, catalogs: DefinitionCatalog): Unit = {

			val container = new MutContainer

			containerStack.push(container)

			val parsed = Utils.readToml(gameName, path, getClass)

			if (parsed.contains("defaults"))
				defaults ++= Utils.toTable(parsed("defaults"))

			// includes
			if (parsed.contains("include")) {
				val tincs = Utils.toArray(parsed("include"))
				for (include <- tincs) {
					val table           = Utils.toTable(include)
					val incResourceName = Utils.toString(table("resource"))
					val incResourcePath = Path.of(incResourceName)

					Utils.readFile(incResourceName, incResourcePath, getClass) match {
						case Some(d) => process(incResourcePath, catalogs)
						case _       =>
							println(s"Include resource file $incResourceName not found")
							containerStack.pop()
							return
					}
				}
			}

			// extract instances
			if (parsed.contains("instance")) {
				val instances = Utils.toArray(parsed("instance"))
				container.children ++= instances.flatMap(Instance(_, defaults.toMap, catalogs))
			}

			// extract connections
			if (parsed.contains("connection")) {
				val connections = Utils.toArray(parsed("connection"))
				container.connections ++= connections.flatMap(Connection(_, catalogs))
			}
		}
	}

}

