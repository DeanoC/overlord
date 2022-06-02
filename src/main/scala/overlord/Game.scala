package overlord

import ikuy_utils._
import overlord.Connections._
import overlord.Instances._
import overlord.Interfaces.{ChipLike, RegisterBankLike}

import java.nio.file.{Files, Path}
import scala.collection.mutable
import scala.reflect.ClassTag

case class Game(name: String,
                children: Seq[InstanceTrait],
                connected: Seq[Connected],
                distanceMatrix: DistanceMatrix,
                wires: Seq[Wire]
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
		children.filter(_.isInstanceOf[ChipInstance]).map(_.asInstanceOf[ChipInstance]) ++
		children.filter(_.isInstanceOf[Container]).map(_.asInstanceOf[Container]).flatMap(_.flatChildren.map(_.asInstanceOf[ChipInstance]))

	lazy val flatSoftwareChildren: Seq[SoftwareInstance] = children.filter(_.isInstanceOf[SoftwareInstance]).map(_.asInstanceOf[SoftwareInstance])

	lazy val allChipInstances: Seq[ChipInstance] = flatChipChildren

	lazy val allSoftwareInstances: Seq[SoftwareInstance] = flatSoftwareChildren

	lazy val cpus: Seq[CpuInstance] = flatChipChildren.filter(_.isInstanceOf[CpuInstance]).map(_.asInstanceOf[CpuInstance])

	lazy val storages: Seq[StorageInstance] = flatChipChildren.filter(_.isInstanceOf[StorageInstance]).map(_.asInstanceOf[StorageInstance])

	lazy val nets: Seq[NetInstance] = flatChipChildren.filter(_.isInstanceOf[NetInstance]).map(_.asInstanceOf[NetInstance])

	lazy val pins: Seq[PinGroupInstance] = flatChipChildren.filter(_.isInstanceOf[PinGroupInstance]).map(_.asInstanceOf[PinGroupInstance])

	lazy val clocks: Seq[ClockInstance] = flatChipChildren.filter(_.isInstanceOf[ClockInstance]).map(_.asInstanceOf[ClockInstance])

	lazy val gatewares: Seq[ChipInstance] = flatChipChildren.filter(_.definition.isInstanceOf[GatewareDefinitionTrait])

	lazy val libraries: Seq[LibraryInstance] = flatSoftwareChildren.filter(_.isInstanceOf[LibraryInstance]).map(_.asInstanceOf[LibraryInstance])

	lazy val programs: Seq[ProgramInstance] = flatSoftwareChildren.filter(_.isInstanceOf[ProgramInstance]).map(_.asInstanceOf[ProgramInstance])

	lazy val constants: Seq[ConnectedConstant] = connected.filter(_.isInstanceOf[ConnectedConstant]).map(_.asInstanceOf[ConnectedConstant])

	lazy val board: Option[BoardInstance] = children.find(_.isInstanceOf[BoardInstance]).asInstanceOf[Option[BoardInstance]]


	def getInterfacesDirectlyConnectedTo[T <: ChipLike](instance: ChipInstance)(implicit tag: ClassTag[T]): Seq[T] =
		flatChipChildren.flatMap(c => c.getInterface[T]).filter(f => distanceMatrix.distanceBetween(instance, f.getOwner) == 1)

	def getInstancesWithInterfaceBetween[T <: ChipLike](start: ChipInstance, end: ChipInstance)(implicit tag: ClassTag[T]): Seq[ChipInstance] =
		getInterfacesConnectedTo[T](start).flatMap(inf => if (distanceMatrix.isConnectedBetween(inf.getOwner, end)) Some(end) else None)

	def getInterfacesConnectedTo[T <: ChipLike](instance: ChipInstance)(implicit tag: ClassTag[T]): Seq[T] =
		flatChipChildren.flatMap(_.getInterface[T]).filter(f => distanceMatrix.isConnectedBetween(instance, f.getOwner))
}

object Game {
	private val catalogPathStack : mutable.Stack[Path] = mutable.Stack[Path]()
	private val instancePathStack: mutable.Stack[Path] = mutable.Stack[Path]()
	private val outPathStack     : mutable.Stack[Path] = mutable.Stack[Path]()

	private var baseProjectPath: Path = Path.of("")

	def setupPaths(projectPath: Path, catalogPath: Path, prefabsPath: Path, outPath: Path): Unit = {
		baseProjectPath = projectPath.toAbsolutePath

		catalogPathStack.clear()
		catalogPathStack.push(catalogPath)
		catalogPathStack.push(catalogPath)

		instancePathStack.clear()
		instancePathStack.push(prefabsPath)
		instancePathStack.push(prefabsPath)

		outPathStack.clear()
		outPathStack.push(outPath)
		outPathStack.push(outPath)
	}

	def projectPath: Path = baseProjectPath

	def pushCatalogPath(path: String): Unit = pushCatalogPath(Path.of(path))

	def pushCatalogPath(path: Path): Unit = {
		val potDPath = catalogPath.resolve(path);
		if (potDPath.toFile.isFile) catalogPathStack.push(potDPath.getParent)
		else catalogPathStack.push(potDPath)
	}

	def catalogPath: Path = catalogPathStack.top

	def setInstancePath(path: String): Unit = setInstancePath(Path.of(path))

	def setInstancePath(path: Path): Unit = {
		instancePathStack.push(path)
	}

	def pushInstancePath(path: String): Unit = pushInstancePath(Path.of(path))

	def pushInstancePath(path: Path): Unit = {
		val potIPath = instancePath.resolve(path);
		if (potIPath.toFile.isFile) instancePathStack.push(potIPath.getParent)
		else instancePathStack.push(potIPath)
	}

	def instancePath: Path = instancePathStack.top

	def pushOutPath(path: String): Unit = pushOutPath(Path.of(path))

	def pushOutPath(path: Path): Unit = {
		val potOPath = outPath.resolve(path);
		if (potOPath.toFile.isFile) outPathStack.push(potOPath.getParent)
		else outPathStack.push(potOPath)
	}

	def outPath: Path = outPathStack.top

	def popCatalogPath(): Unit = catalogPathStack.pop()

	def popInstancePath(): Unit = instancePathStack.pop()

	def popOutPath(): Unit = outPathStack.pop()

	def resolveInstanceMacros(instance: InstanceTrait, inString: String): String = {
		inString
			.replace("${name}", instance.name)
	}

	def resolvePathMacros(instance: InstanceTrait, inString: String): String = {
		resolveInstanceMacros(instance, inString)
			.replace("${projectPath}", projectPath.toString)
			.replace("${definitionPath}", instance.definition.sourcePath.toString)
			.replace("${outPath}", outPath.toString)
			.replace("${instancePath}", instance.sourcePath.toString)
	}

	def tryPaths(instance: InstanceTrait, resource: String): Path = {
		tryPaths(instance, resource, 0)
	}

	def apply(gameName: String,
	          board: String,
	          gamePath: Path,
	          catalogs: DefinitionCatalog,
	          prefabs: PrefabCatalog): Option[Game] = {

		// check for duplicates
		val keyArray = catalogs.catalogs.keys.toArray.sortInPlaceWith {
			_.ident.mkString(".") < _.ident.mkString(".")
		}

		for (i <- 0 until catalogs.catalogs.size) {
			for (j <- i + 1 until catalogs.catalogs.size) {
				if ((keyArray(i).ident == keyArray(j).ident)) {
					println(s"WARN: Duplicate definition name ${keyArray(i).ident} detected")
				}
			}
		}

		val gb = new GameBuilder(gameName, board, gamePath, catalogs, prefabs)
		gb.toGame
	}

	private def tryPaths(instance: InstanceTrait, resource: String, pass: Int): Path = {
		val givenPath = Path.of(resolvePathMacros(instance, resource))
		if (!Files.exists(givenPath)) {
			val instancePath = Path.of(Game.resolvePathMacros(instance, "${instancePath}/" + resource))
			if (!Files.exists(instancePath)) {
				val definitionPath = Path.of(Game.resolvePathMacros(instance, "${definitionPath}/" + resource))
				if (!Files.exists(definitionPath)) {
					val outPath = Path.of(Game.resolvePathMacros(instance, "${outPath}/" + resource))
					if (!Files.exists(outPath)) {
						if (pass == 0 && instance.definition.isInstanceOf[SoftwareDefinitionTrait]) {
							val softDef = instance.definition.asInstanceOf[SoftwareDefinitionTrait]
							tryPaths(instance, softDef.actionsFilePath.getParent.toString + "/" + resource, 1)
						} else {
							println(s"tryPath: $resource file not found")
							Path.of("")
						}
					} else outPath
				} else definitionPath
			} else instancePath
		} else givenPath
	}

	private class GameBuilder(gameName: String,
	                          board: String,
	                          gamePath: Path,
	                          catalogs: DefinitionCatalog,
	                          prefabs: PrefabCatalog) {

		val containerStack: mutable.Stack[Container] = mutable.Stack()

		private val defaults = mutable.Map[String, Variant]()

		def toGame: Option[Game] = {
			generateInstances(gamePath)

			if (containerStack.isEmpty) {
				println("Previous Errors mean game cannot be created\n")
				return None
			}

			val rootContainer = new RootContainer
			injectBoard(rootContainer)
			flattenContainers(rootContainer)

			// add any software dependecies
			val sis          = rootContainer.children.collect { case s: SoftwareInstance => s }
			val allDeps      = (for (si <- sis) yield si.definition.dependencies).flatten.distinct
			val existingLibs = sis.flatMap { inst =>
				if (inst.isInstanceOf[Instances.LibraryInstance]) {
					Some(inst.name)
				} else None
			}
			val depsRequired = allDeps.diff(existingLibs)
			depsRequired.foreach(n => catalogs.findDefinition(
				LibraryDefinitionType(Seq("library", s"$n"))) match {
				case Some(defi) =>
					val opt = defi.createInstance(n, Map[String, Variant]())
					if (opt.nonEmpty) rootContainer.children ++= Seq(opt.get)
					else {
						println(s"ERROR: Software depdency ${defi.defType.ident.mkString(".")}")
					}
				case None       =>
			})

			// connect everything
			val (setOfConnected, distanceMatrix, wires) = connectAndOutputChips(rootContainer)
			outputSoftware(rootContainer, setOfConnected, distanceMatrix)

			// get chips (hardware or gateware)
			val instances = rootContainer.children.collect {
				case i: ChipInstance     => i
				case s: SoftwareInstance => s
			}

			Some(Game(gameName,
			          instances,
			          setOfConnected.toSeq,
			          distanceMatrix,
			          wires))
		}

		private def flattenContainers(rootContainer: RootContainer): Unit = {
			// flatten all containers
			for (c <- containerStack.popAll()) {
				rootContainer.children ++= c.children
				rootContainer.unconnected ++= c.unconnected
			}

			containerStack.clear()
		}

		private def outputSoftware(rootContainer: RootContainer,
		                           setOfConnected: Set[Connected],
		                           distanceMatrix: DistanceMatrix): Unit = {
			pushOutPath("soft/tmp")
			Utils.ensureDirectories(outPath)

			// get software (libraries, boot roms) for all cpus
			val softInstances = rootContainer.children.collect { case i: SoftwareInstance => i }
			val cpuInstances  = rootContainer.children.collect { case i: CpuInstance => i }

			OutputSoftware.cpuInvariantActions(softInstances, setOfConnected.toSeq)
			OutputSoftware.cpuSpecificActions(board, softInstances, cpuInstances, distanceMatrix, setOfConnected.toSeq)
			OutputSoftware.hardwareRegistersOutput(cpuInstances, distanceMatrix, setOfConnected.toSeq)

			popOutPath()
		}

		private def generateInstances(path: Path): Boolean = {
			containerStack.push(RootContainer())
			val parsed = Utils.readToml(path)
			if (parsed.contains("defaults")) defaults ++= Utils.toTable(parsed("defaults"))
			if (!processInstantiations(parsed, containerStack.top)) {
				println(s"Instantiation failed")
				false
			} else true
		}

		private def injectBoard(rootContainer: RootContainer) = {
			// pass the board as if it had been a prefab in the main project file
			val boardInsertV = Map[String, Variant](
				"instance" -> ArrayV(Array(
					TableV(
						Map[String, Variant](
							"name" -> StringV(s"$board"),
							"type" -> StringV(s"board.$board"))
						))),
				"prefab" -> ArrayV(Array(
					TableV(
						Map[String, Variant]("name" -> StringV(s"boards.$board"))
						))))
			processInstantiations(boardInsertV, rootContainer)
		}

		private def connectAndOutputChips(rootContainer: RootContainer) = {
			pushOutPath("gate")
			Utils.ensureDirectories(outPath)

			val chipInstances = rootContainer.children.collect { case i: ChipInstance => i }

			// preconnect for bus address allocations
			rootContainer.unconnected.foreach(_.preConnect(chipInstances))

			// run gateware actions
			OutputGateware(rootContainer, outPath, 1)
			OutputGateware(rootContainer, outPath, 2)

			rootContainer.unconnected.foreach(_.finaliseBuses(chipInstances))

			// connect chips
			val connected = rootContainer.unconnected.flatMap(_.connect(chipInstances))

			// instances that are connected to buses need a register bank
			val registerBanks = connected.flatMap(c => c.getInterface[RegisterBankLike])
			registerBanks.foreach(_.generateInstancedRegisterBank())

			// get connections and compute the distance matrix
			val setOfConnected     = connected.toSet
			val dm: DistanceMatrix = DistanceMatrix(chipInstances, setOfConnected.toSeq)
			val wires              = Wires(dm, setOfConnected.toSeq)
			popOutPath()

			(setOfConnected, dm, wires)

		}

		private def processInstantiations(parsed: Map[String, Variant],
		                                  container: Container): Boolean = {

			// includes
			if (parsed.contains("include")) {
				val tincs = Utils.toArray(parsed("include"))
				for (include <- tincs) {
					val table           = Utils.toTable(include)
					val incResourceName = Utils.toString(table("resource"))
					val incResourcePath = Path.of(incResourceName)

					prefabs.findPrefabInclude(incResourceName) match {
						case Some(value) =>
							Game.setInstancePath(value.path)
							val variants = prefabs.flattenIncludesContents(value.name)
							processInstantiations(variants, container)
							Game.popInstancePath()
						case None        =>
							Utils.readFile(incResourcePath) match {
								case Some(d) =>
									if (!generateInstances(incResourcePath)) return false
								case _       =>
									println(s"Include resource file $incResourceName not found")
									return false
							}
					}
				}
			}

			// extract instances
			if (parsed.contains("instance")) {
				val instancesWanted = Utils.toArray(parsed("instance"))
				val instances       = instancesWanted.flatMap(Instance(_, defaults.toMap, catalogs))
				// do dependencies later when everything is flattened
				container.children ++= instances
			}

			// extract connections
			if (parsed.contains("connection")) {
				val connections = Utils.toArray(parsed("connection"))
				container.unconnected ++= connections.flatMap(Unconnected(_, catalogs))
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
							okay &= processInstantiations(pf.stuff, container)
						case None     =>
							println(s"Error: prefab $ident not found")
							return false
					}
				}
			}
			okay
		}
	}
}

