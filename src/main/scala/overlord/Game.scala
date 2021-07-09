package overlord

import overlord.Connections.{Connected, ConnectedBetween, ConnectedConstant, Connection, ConstantConnectionType}
import overlord.Definitions.{DefinitionTrait, GatewareTrait}
import overlord.Gateware.GatewareAction.GatewareAction
import overlord.Instances._
import ikuy_utils._
import overlord.Software.RegisterBank

import java.nio.file.Path
import scala.collection.mutable

case class Game(name: String,
                override val children: Seq[Instance],
                connections: Seq[Connection],
               ) extends Container {

	override val physical: Boolean = false
	val distanceMatrix: DistanceMatrix = DistanceMatrix(children)

	override def copyMutateContainer(copy: MutContainer): Container =
		Game(name, copy.children.toSeq, copy.connections.toSeq)

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

	lazy val peripherals: Seq[Instance] = storages ++ nets

	lazy val gatewares: Seq[(Instance, GatewareTrait)] =
		flatChildren.filter(_.definition.gateware.nonEmpty)
			.map(g => (g, g.definition.gateware.get))

	lazy val constants: Seq[ConnectedConstant] =
		connections.filter(_.isInstanceOf[ConnectedConstant])
			.map(_.asInstanceOf[ConnectedConstant])

	lazy val board: Option[BoardInstance] =
		children.find(_.isInstanceOf[BoardInstance])
			.asInstanceOf[Option[BoardInstance]]

	lazy val connected: Seq[Connected] =
		connections.filter(_.isInstanceOf[Connected])
			.map(_.asInstanceOf[Connected])
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

			// inform instances that are connected to busses, there address
			val buses = expanded.filter(_.isInstanceOf[BusInstance])
				.map(_.asInstanceOf[BusInstance])
			for(  bus <- buses;
						inst <- bus.connectedInstances;
						rl <- inst.instanceRegisterLists
			      if !inst.instanceRegisterBanks.exists(_.name == rl.name)) {

				val (address, _) = bus.getConsumerAddressAndSize(inst)
				inst.instanceRegisterBanks +=
					RegisterBank(s"${bus.ident}_${inst.ident}",address, rl.name)
			}


			Some(Game(gameName, expanded, expandedConnections))
		}
	}

}

object OutputGateware {
	def apply(top: MutContainer,
	          gatePath: Path,
	          phase: GatewareAction => Boolean): Unit = {
		Game.pathStack.push(gatePath.toRealPath())

		for {(instance, gateware) <-
			     top.children.filter(_.definition.gateware.nonEmpty)
				     .map(g => (g, g.definition.gateware.get))} {

			OutputGateware.executePhase(instance, gateware,
			                            top.connections.toSeq, phase)
		}
	}

	def executePhase(instance: Instance,
	                 gateware: GatewareTrait,
	                 connections: Seq[Connection],
	                 phase: GatewareAction => Boolean): Unit = {

		val backupStack = Game.pathStack.clone()
		for {action <- gateware.actions.filter(phase(_))} {

			val conParameters = connections
				.filter(_.isUnconnected)
				.map(_.asUnconnected)
				.filter(_.isConstant).map(c => {
				val constant = c.connectionType.asInstanceOf[ConstantConnectionType]
				val name     = c.secondFullName.split('.').lastOption match {
					case Some(value) => value
					case None        => c.secondFullName
				}
				mutable.Map[String, Variant](name -> constant.constant)
			}).fold(mutable.HashMap[String, Variant]())((o, n) => o ++ n)

			val instanceSpecificParameters = instance match {
				case bus: BusInstance =>
					Map[String, Variant]("consumers" -> bus.consumersVariant)
				case ram: RamInstance =>
					ram.sizeInBytes match {
						case Some(v) =>
							Map[String, Variant]("size_in_bytes" -> BigIntV(v))
						case None    => Map[String, Variant]()
					}
				case _                => Map[String, Variant]()
			}

			val parameters = instance.parameters ++
			                 conParameters ++
			                 instanceSpecificParameters

			val merged = connections.filter(
				_.isInstanceOf[ConnectedConstant])
				.map(_.asInstanceOf[ConnectedConstant])
				.filter(c => instance.parameterKeys.contains(c.secondFullName))
				.map(_.asParameter)
				.fold(parameters)((o, n) => o ++ n)

			action.execute(instance, merged, Game.pathStack.top)
		}

		Game.pathStack = backupStack
	}
}