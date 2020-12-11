package overlord

import overlord.Connections.{Connected, ConnectedBetween, ConnectedConstant, Connection, ConstantConnectionType}
import overlord.Definitions.GatewareTrait
import overlord.Gateware.GatewareAction.GatewareAction
import overlord.Gateware.Parameter
import overlord.Instances._
import toml.Value

import java.nio.file.{Files, Path}
import scala.collection.mutable
import scala.util.{Failure, Success, Try}

case class Game(name: String,
                override val children: Seq[Instance],
                connections: Seq[Connection],
               ) extends Container {

	override val physical: Boolean = false
	val distanceMatrix: DistanceMatrix = DistanceMatrix(children)

	override def copyMutateContainer(copy: MutContainer): Container =
		Game(name, copy.children.toSeq, copy.connections.toSeq)


	lazy val cpus: Seq[CpuInstance] = flatChildren
		.filter(_.isInstanceOf[CpuInstance])
			.map(_.asInstanceOf[CpuInstance])

	lazy val rams: Seq[RamInstance] =
		flatChildren.filter(_.isInstanceOf[RamInstance])
			.map(_.asInstanceOf[RamInstance])

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

	def apply(gamePath: String,
	          gameText: String,
	          out: Path,
	          catalogs: DefinitionCatalog): Option[Game] = {
		val gameName = gamePath.split('/').last.split('.').head
		val gb       = new GameBuilder(gameName, gameText, catalogs)
		gb.toGame(out)
	}

	private def readFile(name: String): Option[String] = {
		println(s"Reading $name")

		val path = Game.pathStack.top.resolve(name)

		if (!Files.exists(path.toAbsolutePath)) {
			// try resource
			Try(getClass.getResourceAsStream("/" + name)) match {
				case Failure(exception) =>
					println(s"$name catalog at ${name} $exception");
					None
				case Success(value)     =>
					if (value == null) {
						println(s"$name catalog at ${name} not found");
						None
					}
					Some(scala.io.Source.fromInputStream(value).mkString)
			}
		} else {
			val file   = path.toAbsolutePath.toFile
			val source = scala.io.Source.fromFile(file)
			Some(source.getLines().mkString("\n"))
		}
	}

	private class GameBuilder(gameName: String,
	                          gameText: String,
	                          catalogs: DefinitionCatalog) {

		private val defaults = mutable.Map[String, Value]()

		containerStack.push(new MutContainer)

		process(gameText, catalogs)

		private def process(data: String, catalogs: DefinitionCatalog): Unit = {

			val parsed = {
				val parsed = toml.Toml.parse(data)
				parsed match {
					case Right(value) => value.values
					case Left(_)      => println(s"game.over has failed to parse with " +
					                             s"error ${Left(parsed)}")
						return
				}
			}

			if (parsed.contains("defaults"))
				defaults ++= Utils.toTable(parsed("defaults"))

			// includes
			if (parsed.contains("include")) {
				val tincs = Utils.toArray(parsed("include"))
				for (include <- tincs) {
					val table       = Utils.toTable(include)
					val incResource = Utils.toString(table("resource"))

					containerStack.push(new MutContainer)

					Game.readFile(incResource) match {
						case Some(d) => process(d, catalogs)
						case _       =>
							println(s"Include resource file ${incResource} not found")
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
		           phase1: Boolean = true,
		           phase2: Boolean = true): Option[Game] = {
			val softPath = out.resolve("soft")
			val gatePath = out.resolve("gate")
			Utils.ensureDirectories(softPath)
			Utils.ensureDirectories(gatePath)

			val top = containerStack.top

			if (phase1) {
				println("Procesing gateware phase 1")
				executePhase(top.children.toSeq, top.connections.toSeq, gatePath,
					{
						_.isPhase1
					})
			}

			if (phase2) {
				println("Procesing gateware phase 2")
				executePhase(top.children.toSeq, top.connections.toSeq, gatePath,
					{
						_.isPhase2
					})
			}

			val (expanded, expandedConnections) =
				Connection.expandAndConnect(top.connections.toSeq, top.children.toSeq)

			Some(Game(gameName, expanded, expandedConnections))
		}
	}

	private def executePhase(instances: Seq[Instance],
	                         connections: Seq[Connection],
	                         gatePath: Path,
	                         phase: (GatewareAction) => Boolean): Unit = {
		Game.pathStack.push(gatePath.toRealPath())
		for {(gateware, defi) <-
			     instances.filter(_.definition.gateware.nonEmpty)
				     .map(g => (g, g.definition.gateware.get))} {
			val backupStack = Game.pathStack.clone()

			for {action <- defi.actions.filter(phase(_))} {

				val conParameters  = connections
					.filter(_.isUnconnected)
					.map(_.asUnconnected)
					.filter(_.isConstant).map(c => {
					val constant = c.connectionType.asInstanceOf[ConstantConnectionType]
					val name     = c.secondFullName.split('.').lastOption match {
						case Some(value) => value
						case None        => c.secondFullName
					}
					mutable.HashMap[String, Parameter](
						(name, Parameter(name, constant.constant)))
				}).fold(mutable.HashMap[String, Parameter]())((o, n) => o ++ n)

				val parameters = defi.parameters ++ conParameters
				val merged     = connections.filter(
					_.isInstanceOf[ConnectedConstant])
					.map(_.asInstanceOf[ConnectedConstant])
					.map(_.asParameter)
					.fold(parameters)((o, n) => o ++ n)

				action.execute(gateware, merged.toMap, Game.pathStack.top)
			}
			Game.pathStack = backupStack
		}
	}
}