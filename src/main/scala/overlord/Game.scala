package overlord

import overlord.Connections.{Connected, ConnectedConstant, Connection, Wire, Wires}
import overlord.Definitions.GatewareTrait
import overlord.Instances._
import ikuy_utils._
import overlord.Software.RegisterBank

import java.nio.file.Path
import scala.collection.mutable

case class Game(name: String,
                children: Seq[Instance],
                connected: Seq[Connected],
                distanceMatrix: DistanceMatrix,
                wires: Seq[Wire]
               ){
	lazy val setOfGateware: Set[Instance] = {
		val setOfGateware = mutable.HashSet[Instance]()
		connected.foreach { c =>
			if (c.first.nonEmpty && c.first.get.instance.isGateware)
				setOfGateware += c.first.get.instance
			if (c.second.nonEmpty && c.second.get.instance.isGateware)
				setOfGateware += c.second.get.instance
		}
		setOfGateware.toSet
	}

	lazy val flatChildren: Seq[Instance] =
		children.filter(_.isInstanceOf[Container])
			.map(_.asInstanceOf[Container]).flatMap(_.flatChildren) ++ children

	lazy val allInstances: Seq[Instance] = flatChildren

	lazy val cpus: Seq[CpuInstance] = flatChildren
		.filter(_.isInstanceOf[CpuInstance])
		.map(_.asInstanceOf[CpuInstance])

	lazy val rams: Seq[RamInstance] =
		flatChildren.filter(_.isInstanceOf[RamInstance])
			.map(_.asInstanceOf[RamInstance])

	lazy val buses: Seq[BusInstance] =
		flatChildren.filter(_.isInstanceOf[BusInstance])
			.map(_.asInstanceOf[BusInstance])

	lazy val storages: Seq[StorageInstance] =
		flatChildren.filter(_.isInstanceOf[StorageInstance])
			.map(_.asInstanceOf[StorageInstance])

	lazy val nets: Seq[NetInstance] =
		flatChildren.filter(_.isInstanceOf[NetInstance])
			.map(_.asInstanceOf[NetInstance])

	lazy val pins: Seq[PinGroupInstance] =
		flatChildren.filter(_.isInstanceOf[PinGroupInstance])
			.map(_.asInstanceOf[PinGroupInstance])

	lazy val clocks: Seq[ClockInstance] =
		flatChildren.filter(_.isInstanceOf[ClockInstance])
			.map(_.asInstanceOf[ClockInstance])

	lazy val gatewares: Seq[(Instance, GatewareTrait)] =
		flatChildren.filter(_.definition.gateware.nonEmpty)
			.map(g => (g, g.definition.gateware.get))

	lazy val constants: Seq[ConnectedConstant] =
		connected.filter(_.isInstanceOf[ConnectedConstant])
			.map(_.asInstanceOf[ConnectedConstant])

	lazy val board: Option[BoardInstance] =
		children.find(_.isInstanceOf[BoardInstance])
			.asInstanceOf[Option[BoardInstance]]

	def getBusesConnectedToCpu(cpu:CpuInstance) : Seq[BusInstance] = {
		buses.filter(_.consumerInstances.contains(cpu))
	}
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

		containerStack.push(new MutContainer)

		process(gamePath, catalogs)

		private def process(path: Path, catalogs: DefinitionCatalog): Unit = {

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

					containerStack.push(new MutContainer)

					Utils.readFile(incResourceName, incResourcePath, getClass) match {
						case Some(d) => process(incResourcePath, catalogs)
						case _       =>
							println(s"Include resource file $incResourceName not found")
							containerStack.pop()
							return
					}
				}
			}

			val container = containerStack.top

			// extract instances
			if (parsed.contains("instance")) {
				val instances = Utils.toArray(parsed("instance"))
				container.children ++= instances.flatMap(
					Instance(_, defaults.toMap, catalogs))
			}

			// extract connections
			if (parsed.contains("connection")) {
				val connections = Utils.toArray(parsed("connection"))
				container.connections ++=
				connections.flatMap(Connection(_, catalogs))
			}
			// find instance to use as a container
			container.children.find(
				i => i.isInstanceOf[Container] &&
				     i.asInstanceOf[Container].children.isEmpty) match {
				case Some(v) =>
					val c = v.asInstanceOf[Container]
					container.children = container.children
						.filterNot(_.isInstanceOf[Container])
					val n = c.copyMutateContainer(container)
					containerStack.pop()
					val t = Game.containerStack.top
					t.children ++= Seq(n.asInstanceOf[Instance])

				case None =>
			}
		}

		def toGame(out: Path,
		           doPhase1: Boolean = true,
		           doPhase2: Boolean = true): Option[Game] = {
			val softPath = out.resolve("soft")
			val gatePath = out.resolve("gate")
			Utils.ensureDirectories(softPath)
			Utils.ensureDirectories(gatePath)

			val top = containerStack.top

			Connection.preConnect(top.connections.toSeq, top.children.toSeq)

			OutputGateware(top, gatePath, {
				_.isPhase1
			})

			OutputGateware(top, gatePath, {
				_.isPhase2
			})

			val (expanded, expandedConnections) =
				Connection.expandAndConnect(top.connections.toSeq, top.children.toSeq)

			// instances that are connected to buses need a register bank
			val buses = expanded.filter(_.isInstanceOf[BusInstance])
				.map(_.asInstanceOf[BusInstance])
			for(bus <- buses;
			    inst <- bus.consumerInstances;
			    rl <- inst.instanceRegisterLists
			    if !inst.instanceRegisterBanks.exists(_.name == rl.name)) {

				val (address, _) = bus.getConsumerAddressAndSize(inst)
				inst.instanceRegisterBanks +=
					RegisterBank(s"${bus.ident}_${inst.ident}",address, rl.name)
			}

			val setOfConnected = expandedConnections
				.filter(_.isConnected)
				.map(_.asConnected).toSet

			val dm: DistanceMatrix = DistanceMatrix(expanded)

			val connectionMask = Array.fill[Boolean](dm.dim)(elem = false)
			for {connected <- setOfConnected
			     (sp, ep) = dm.indicesOf(connected)} {
				connectionMask(sp) = true
				dm.routeBetween(sp, ep).foreach(connectionMask(_) = true)
			}
			dm.removeSelfLinks()
			dm.instanceMask(connectionMask)

			val wires = Wires(dm, setOfConnected.toSeq)

			Some(Game(gameName, expanded, setOfConnected.toSeq, dm, wires))
		}
	}

}

