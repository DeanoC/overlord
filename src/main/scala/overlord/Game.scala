package overlord

import ikuy_utils._
import overlord.Chip.RegisterBank
import overlord.Connections._
import overlord.Instances._

import java.nio.file.Path
import scala.collection.mutable

case class Game(name: String,
                children: Seq[InstanceTrait],
                connected: Seq[Connected],
                distanceMatrix: DistanceMatrix,
                wires: Seq[Wire],
                out: Path
               ) {
	lazy val setOfConnectedGateware: Set[ChipInstance] = {
		val setOfGateware = mutable.HashSet[ChipInstance]()
		connected.foreach { c =>
			if (c.first.nonEmpty && c.first.get.isGateware)
				setOfGateware += c.first.get.instance.asInstanceOf[ChipInstance]
			if (c.second.nonEmpty && c.second.get.isGateware)
				setOfGateware += c.second.get.instance.asInstanceOf[ChipInstance]
		}
		setOfGateware.toSet
	}

	lazy val setOfConnectedSoftware: Set[SoftwareInstance] = {
		val setOfSoftware = mutable.HashSet[SoftwareInstance]()
		connected.foreach { c =>
			if (c.first.nonEmpty && c.first.get.isSoftware)
				setOfSoftware += c.first.get.instance.asInstanceOf[SoftwareInstance]
			if (c.second.nonEmpty && c.second.get.isSoftware)
				setOfSoftware += c.second.get.instance.asInstanceOf[SoftwareInstance]
		}
		setOfSoftware.toSet
	}

	lazy val flatChipChildren: Seq[ChipInstance] =
		children
			.filter(_.isInstanceOf[ChipInstance])
			.map(_.asInstanceOf[ChipInstance]) ++
		children.filter(_.isInstanceOf[Container])
			.map(_.asInstanceOf[Container])
			.flatMap(_.flatChildren.map(_.asInstanceOf[ChipInstance]))

	lazy val flatSoftwareChildren: Seq[SoftwareInstance] =
		children
			.filter(_.isInstanceOf[SoftwareInstance])
			.map(_.asInstanceOf[SoftwareInstance])

	lazy val allChipInstances: Seq[ChipInstance] = flatChipChildren

	lazy val allSoftwareInstances: Seq[SoftwareInstance] = flatSoftwareChildren

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

	lazy val libraries: Seq[LibraryInstance] =
		flatSoftwareChildren.filter(_.isInstanceOf[LibraryInstance])
			.map(_.asInstanceOf[LibraryInstance])

	lazy val programs: Seq[ProgramInstance] =
		flatSoftwareChildren.filter(_.isInstanceOf[ProgramInstance])
			.map(_.asInstanceOf[ProgramInstance])

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


	private val tmp = out
		.resolve("soft")
		.resolve("build")
		.resolve("tmp")

	Utils.ensureDirectories(tmp)
	OutputSoftware.hardwareRegistersOutput(this, tmp)
	OutputSoftware.cpuInvariantActions(this, tmp)
	OutputSoftware.cpuSpecificActions(this, tmp)
}

object Game {
	// these are mutable for easy backup and restore
	var pathStack     : mutable.Stack[Path]      = mutable.Stack()
	var containerStack: mutable.Stack[Container] = mutable.Stack()

	def apply(gameName: String,
	          board: String,
	          gamePath: Path,
	          out: Path,
	          catalogs: DefinitionCatalog,
	          prefabs: PrefabCatalog): Option[Game] = {
		val gb   = new GameBuilder(gameName, board, gamePath, catalogs, prefabs)
		val game = gb.toGame(out)

		game
	}

	private class GameBuilder(gameName: String,
	                          board: String,
	                          gamePath: Path,
	                          catalogs: DefinitionCatalog,
	                          prefabs: PrefabCatalog) {

		private val defaults = mutable.Map[String, Variant]()
		process(gamePath, catalogs, prefabs)

		def toGame(out: Path): Option[Game] = {
			val softPath = out.resolve("soft")
			Utils.ensureDirectories(softPath)

			if (containerStack.isEmpty) {
				println("Previous Errors mean game cannot be created\n")
				return None
			}
			val rootContainer = new RootContainer

			// pass the board as if it had been a prefab in the main project file
			val boardInsertV = Map[String, Variant](
				("instance" -> ArrayV(Array(
					TableV(
						Map[String, Variant](
							"name" -> StringV(s"$board"),
							"type" -> StringV(s"board.$board"))
						)))
					),
				("prefab" -> ArrayV(Array(
					TableV(
						Map[String, Variant]("name" -> StringV(s"boards.$board"))
						)))
					))
			ProcessInstantiation(boardInsertV, rootContainer)
			// flatten all containers
			for (c <- containerStack.popAll()) {
				rootContainer.children ++= c.children
				rootContainer.connections ++= c.connections
			}
			containerStack.clear()
			val top = rootContainer

			// get chips (hardware or gateware)
			val chipInstances               = top.children.
				filter(_.isInstanceOf[ChipInstance]).
				map(_.asInstanceOf[ChipInstance])
			val (setOfConnected, dm, wires) = DoChips(chipInstances, out, top)

			// get software (libraries, boot rooms)
			val softInstances = top.children.
				filter(_.isInstanceOf[SoftwareInstance]).
				map(_.asInstanceOf[SoftwareInstance])

			Some(Game(gameName,
			          chipInstances ++ softInstances,
			          setOfConnected.toSeq,
			          dm,
			          wires,
			          out))
		}

		private def DoChips(chipInstances: Seq[ChipInstance],
		                    out: Path,
		                    top: Container) = {
			val gatePath = out.resolve("gate")
			Utils.ensureDirectories(gatePath)

			// preconnect for bus address allocations
			Connection.preConnect(top.connections.toSeq, chipInstances)

			// run gateware actions
			OutputGateware(top, gatePath, 1)
			OutputGateware(top, gatePath, 2)

			// connect chips
			val connected = Connection.connect(top.connections.toSeq, chipInstances)

			// instances that are connected to buses need a register bank
			val buses = chipInstances.filter(_.isInstanceOf[BusInstance])
				.map(_.asInstanceOf[BusInstance])

			for (bus <- buses;
			     inst <- bus.consumerInstances;
			     rl <- inst.instanceRegisterLists
			     if !inst.instanceRegisterBanks.exists(_.name == rl.name)) {

				val (address, _) = bus.getFirstConsumerAddressAndSize(inst)
				inst.instanceRegisterBanks +=
				RegisterBank(s"${bus.ident}_${inst.ident}", address, rl.name)
			}

			// get connections to pass into the distance matrix
			val setOfConnected = connected
				.filter(_.isConnected)
				.map(_.asConnected).toSet

			// produce  distance matrix and wires
			val dm: DistanceMatrix = DistanceMatrix(chipInstances, setOfConnected.toSeq)
			val wires              = Wires(dm, setOfConnected.toSeq)
			(setOfConnected, dm, wires)
		}

		private def ProcessInstantiation(parsed: Map[String, Variant],
		                                 container: Container): Boolean = {

			val includePath = if (parsed.contains("path")) {
				Utils.toString(parsed("path"))
			} else "."

			// includes
			if (parsed.contains("include")) {
				val tincs = Utils.toArray(parsed("include"))
				for (include <- tincs) {
					val table           = Utils.toTable(include)
					val incResourceName = Utils.toString(table("resource"))
					val incResourcePath = Path.of(includePath).resolve(incResourceName)

					Utils.readFile(incResourceName, incResourcePath, getClass) match {
						case Some(d) => process(incResourcePath, catalogs, prefabs)
						case _       =>
							println(s"Include resource file $incResourceName not found")
							containerStack.pop()
							return false
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

			var okay = true

			// bring in wanted prefabs
			if (parsed.contains("prefab")) {
				val tpfs = Utils.toArray(parsed("prefab"))
				for (prefab <- tpfs) {
					val table = Utils.toTable(prefab)
					val ident = Utils.toString(table("name"))
					prefabs.findPrefab(ident) match {
						case Some(pf) =>
							okay &= ProcessInstantiation(pf.instances, container)
						case None     =>
							println(s"prefab $ident not found")
							containerStack.pop()
							return false
					}
				}
			}
			okay
		}

		private def process(path: Path,
		                    catalogs: DefinitionCatalog,
		                    prefabs: PrefabCatalog): Unit = {

			val container: RootContainer = new RootContainer

			containerStack.push(container.asInstanceOf[Container])

			val parsed = Utils.readToml(gameName, path, getClass)

			if (parsed.contains("defaults"))
				defaults ++= Utils.toTable(parsed("defaults"))

			if (!ProcessInstantiation(parsed, container)) {
				println(s"Instantiation failed")
			}
		}
	}
}

