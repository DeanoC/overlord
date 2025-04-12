package com.deanoc.overlord

import com.deanoc.overlord.Connections._
import com.deanoc.overlord.Instances._
import com.deanoc.overlord.Interfaces.{ChipLike, RegisterBankLike}
import com.deanoc.overlord.utils.{Utils, Variant, ArrayV, TableV, StringV}

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable
import scala.reflect.ClassTag
import scala.util.boundary, boundary.break

/** Represents a project in the Overlord system, containing instances,
  * connections, constants, and more. Provides utility methods for managing and
  * interacting with the project structure.
  */
case class Project(
    name: String,
    children: Seq[InstanceTrait],
    connected: Seq[Connected],
    constants: Seq[Constant],
    distanceMatrix: DistanceMatrix,
    busDistanceMatrix: DistanceMatrix,
    wires: Seq[Wire]
) {
  // Lazy evaluation of connected gateware instances.
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

  // Lazy evaluation of connected software instances.
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
      children
        .filter(_.isInstanceOf[Container])
        .map(_.asInstanceOf[Container])
        .flatMap(_.flatChildren.map(_.asInstanceOf[ChipInstance]))

  lazy val flatSoftwareChildren: Seq[SoftwareInstance] = children
    .filter(_.isInstanceOf[SoftwareInstance])
    .map(_.asInstanceOf[SoftwareInstance])

  lazy val allChipInstances: Seq[ChipInstance] = flatChipChildren

  lazy val allSoftwareInstances: Seq[SoftwareInstance] = flatSoftwareChildren

  lazy val cpus: Seq[CpuInstance] = flatChipChildren
    .filter(_.isInstanceOf[CpuInstance])
    .map(_.asInstanceOf[CpuInstance])

  lazy val storages: Seq[StorageInstance] = flatChipChildren
    .filter(_.isInstanceOf[StorageInstance])
    .map(_.asInstanceOf[StorageInstance])

  lazy val nets: Seq[NetInstance] = flatChipChildren
    .filter(_.isInstanceOf[NetInstance])
    .map(_.asInstanceOf[NetInstance])

  lazy val pins: Seq[PinGroupInstance] = flatChipChildren
    .filter(_.isInstanceOf[PinGroupInstance])
    .map(_.asInstanceOf[PinGroupInstance])

  lazy val clocks: Seq[ClockInstance] = flatChipChildren
    .filter(_.isInstanceOf[ClockInstance])
    .map(_.asInstanceOf[ClockInstance])

  lazy val gatewares: Seq[ChipInstance] =
    flatChipChildren.filter(_.definition.isInstanceOf[GatewareDefinitionTrait])

  lazy val libraries: Seq[LibraryInstance] = flatSoftwareChildren
    .filter(_.isInstanceOf[LibraryInstance])
    .map(_.asInstanceOf[LibraryInstance])

  lazy val programs: Seq[ProgramInstance] = flatSoftwareChildren
    .filter(_.isInstanceOf[ProgramInstance])
    .map(_.asInstanceOf[ProgramInstance])

  lazy val board: Option[BoardInstance] = children
    .find(_.isInstanceOf[BoardInstance])
    .asInstanceOf[Option[BoardInstance]]

  /** Retrieves interfaces of a specific type directly connected to a given chip
    * instance.
    *
    * @param instance
    *   The chip instance to check connections for.
    * @tparam T
    *   The type of interface to retrieve.
    * @return
    *   A sequence of interfaces of type T directly connected to the instance.
    */
  def getInterfacesDirectlyConnectedTo[T <: ChipLike](
      instance: ChipInstance
  )(implicit tag: ClassTag[T]): Seq[T] =
    flatChipChildren
      .flatMap(c => c.getInterface[T])
      .filter(f => distanceMatrix.distanceBetween(instance, f.getOwner) == 1)

  /** Retrieves instances with a specific type of interface between two chip
    * instances.
    *
    * @param start
    *   The starting chip instance.
    * @param end
    *   The ending chip instance.
    * @tparam T
    *   The type of interface to check for.
    * @return
    *   A sequence of chip instances with the specified interface type between
    *   the start and end instances.
    */
  def getInstancesWithInterfaceBetween[T <: ChipLike](
      start: ChipInstance,
      end: ChipInstance
  )(implicit tag: ClassTag[T]): Seq[ChipInstance] =
    getInterfacesConnectedTo[T](start).flatMap(inf =>
      if (distanceMatrix.isConnectedBetween(inf.getOwner, end)) Some(end)
      else None
    )

  /** Retrieves interfaces of a specific type connected to a given chip
    * instance.
    *
    * @param instance
    *   The chip instance to check connections for.
    * @tparam T
    *   The type of interface to retrieve.
    * @return
    *   A sequence of interfaces of type T connected to the instance.
    */
  def getInterfacesConnectedTo[T <: ChipLike](
      instance: ChipInstance
  )(implicit tag: ClassTag[T]): Seq[T] =
    flatChipChildren
      .flatMap(_.getInterface[T])
      .filter(f => distanceMatrix.isConnectedBetween(instance, f.getOwner))
}

object Project {
  // Stacks for managing paths during project setup and execution.
  private val catalogPathStack: mutable.Stack[Path] = mutable.Stack[Path]()
  private val instancePathStack: mutable.Stack[Path] = mutable.Stack[Path]()
  private val outPathStack: mutable.Stack[Path] = mutable.Stack[Path]()

  private var baseProjectPath: Path = Paths.get("")

  /** Sets up the paths for the project, including catalog, prefab, and output
    * paths.
    *
    * @param projectPath
    *   The base path of the project.
    * @param catalogPath
    *   The path to the catalog directory.
    * @param prefabsPath
    *   The path to the prefabs directory.
    * @param outPath
    *   The path to the output directory.
    */
  def setupPaths(
      projectPath: Path,
      catalogPath: Path,
      prefabsPath: Path,
      outPath: Path
  ): Unit = {
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

  /** Retrieves the base project path.
    *
    * @return
    *   The base project path.
    */
  def projectPath: Path = baseProjectPath

  /** Pushes a new catalog path onto the stack.
    *
    * @param path
    *   The new catalog path to push.
    */
  def pushCatalogPath(path: String): Unit = pushCatalogPath(Paths.get(path))

  /** Pushes a new catalog path onto the stack.
    *
    * @param path
    *   The new catalog path to push.
    */
  def pushCatalogPath(path: Path): Unit = {
    val potDPath = catalogPath.resolve(path)
    if (potDPath.toFile.isFile) catalogPathStack.push(potDPath.getParent)
    else catalogPathStack.push(potDPath)
  }

  /** Retrieves the current catalog path from the stack.
    *
    * @return
    *   The current catalog path.
    */
  def catalogPath: Path = catalogPathStack.top

  /** Sets the instance path.
    *
    * @param path
    *   The new instance path to set.
    */
  def setInstancePath(path: String): Unit = setInstancePath(Paths.get(path))

  /** Sets the instance path.
    *
    * @param path
    *   The new instance path to set.
    */
  def setInstancePath(path: Path): Unit = {
    instancePathStack.push(path)
  }

  /** Pushes a new instance path onto the stack.
    *
    * @param path
    *   The new instance path to push.
    */
  def pushInstancePath(path: String): Unit = pushInstancePath(Paths.get(path))

  /** Pushes a new instance path onto the stack.
    *
    * @param path
    *   The new instance path to push.
    */
  def pushInstancePath(path: Path): Unit = {
    val potIPath = instancePath.resolve(path)
    if (potIPath.toFile.isFile) instancePathStack.push(potIPath.getParent)
    else instancePathStack.push(potIPath)
  }

  /** Retrieves the current instance path from the stack.
    *
    * @return
    *   The current instance path.
    */
  def instancePath: Path = instancePathStack.top

  /** Pushes a new output path onto the stack.
    *
    * @param path
    *   The new output path to push.
    */
  def pushOutPath(path: String): Unit = pushOutPath(Paths.get(path))

  /** Pushes a new output path onto the stack.
    *
    * @param path
    *   The new output path to push.
    */
  def pushOutPath(path: Path): Unit = {
    val potOPath = outPath.resolve(path)
    if (potOPath.toFile.isFile) outPathStack.push(potOPath.getParent)
    else outPathStack.push(potOPath)
  }

  /** Retrieves the current output path from the stack.
    *
    * @return
    *   The current output path.
    */
  def outPath: Path = outPathStack.top

  /** Pops the top catalog path from the stack.
    */
  def popCatalogPath(): Unit = catalogPathStack.pop()

  /** Pops the top instance path from the stack.
    */
  def popInstancePath(): Unit = instancePathStack.pop()

  /** Pops the top output path from the stack.
    */
  def popOutPath(): Unit = outPathStack.pop()

  /** Resolves instance-specific macros in a given string.
    *
    * @param instance
    *   The instance to use for macro resolution.
    * @param inString
    *   The input string containing macros.
    * @return
    *   The string with resolved macros.
    */
  def resolveInstanceMacros(
      instance: InstanceTrait,
      inString: String
  ): String = {
    inString
      .replace("${name}", instance.name)
  }

  /** Resolves path-related macros in a given string.
    *
    * @param instance
    *   The instance to use for macro resolution.
    * @param inString
    *   The input string containing macros.
    * @return
    *   The string with resolved macros.
    */
  def resolvePathMacros(instance: InstanceTrait, inString: String): String = {
    resolveInstanceMacros(instance, inString)
      .replace("${projectPath}", projectPath.toString)
      .replace("${definitionPath}", instance.definition.sourcePath.toString)
      .replace("${outPath}", outPath.toString)
      .replace("${instancePath}", instance.sourcePath.toString)
  }

  /** Attempts to resolve a resource path for a given instance.
    *
    * @param instance
    *   The instance to use for path resolution.
    * @param resource
    *   The resource to resolve the path for.
    * @return
    *   The resolved path.
    */
  def tryPaths(instance: InstanceTrait, resource: String): Path = {
    tryPaths(instance, resource, 0)
  }

  /** Creates an Project instance from the provided parameters.
    *
    * @param gameName
    *   The name of the project.
    * @param board
    *   The board name.
    * @param gamePath
    *   The path to the project directory.
    * @param catalogs
    *   The definition catalogs.
    * @param prefabs
    *   The prefab catalogs.
    * @return
    *   An optional Project instance.
    */
  def apply(
      gameName: String,
      board: String,
      gamePath: Path,
      catalogs: DefinitionCatalog,
      prefabs: PrefabCatalog
  ): Option[Project] = {

    // check for duplicates
    val keyArray = catalogs.catalogs.keys.toArray.sortInPlaceWith {
      _.ident.mkString(".") < _.ident.mkString(".")
    }

    for (i <- 0 until catalogs.catalogs.size) {
      for (j <- i + 1 until catalogs.catalogs.size) {
        if ((keyArray(i).ident == keyArray(j).ident)) {
          println(
            s"WARN: Duplicate definition name ${keyArray(i).ident} detected"
          )
        }
      }
    }

    val gb = new GameBuilder(gameName, board, gamePath, catalogs, prefabs)
    gb.toGame()
  }

  private def tryPaths(
      instance: InstanceTrait,
      resource: String,
      pass: Int
  ): Path = {
    val givenPath = Paths.get(resolvePathMacros(instance, resource))
    if (!Files.exists(givenPath)) {
      val instancePath = Paths
        .get(Project.resolvePathMacros(instance, "${instancePath}/" + resource))
        .normalize()
      if (!Files.exists(instancePath)) {
        val definitionPath = Paths
          .get(
            Project.resolvePathMacros(instance, "${definitionPath}/" + resource)
          )
          .normalize()
        if (!Files.exists(definitionPath)) {
          val outPath = Paths
            .get(Project.resolvePathMacros(instance, "${outPath}/" + resource))
            .normalize()
          if (!Files.exists(outPath)) {
            if (
              pass == 0 && instance.definition
                .isInstanceOf[SoftwareDefinitionTrait]
            ) {
              val softDef =
                instance.definition.asInstanceOf[SoftwareDefinitionTrait]
              tryPaths(
                instance,
                softDef.actionsFilePath.getParent.toString + "/" + resource,
                1
              )
            } else {
              println(s"tryPath: ${givenPath.toAbsolutePath} file not found")
              Paths.get("")
            }
          } else outPath
        } else definitionPath
      } else instancePath
    } else givenPath
  }

  private class GameBuilder(
      gameName: String,
      board: String,
      gamePath: Path,
      catalogs: DefinitionCatalog,
      prefabs: PrefabCatalog
  ) {

    val containerStack: mutable.Stack[Container] = mutable.Stack()

    private val defaults = mutable.Map[String, Variant]()

    def toGame(): Option[Project] = {
      generateInstances(gamePath)

      if (containerStack.isEmpty) {
        println("Previous Errors mean game cannot be created\n")
        return None
      }

      val rootContainer = new MutableContainer
      injectBoard(rootContainer)
      flattenContainers(rootContainer)

      resolveSoftwareDependencies(rootContainer)

      // connect everything
      val (connected, distanceMatrix, busDistanceMatrix, wires) =
        connectAndOutputChips(rootContainer)
      outputSoftware(rootContainer, connected, busDistanceMatrix)

      // get chips (hardware or gateware)
      val instances = rootContainer.children.collect {
        case i: ChipInstance     => i
        case s: SoftwareInstance => s
      }

      val constants =
        rootContainer.unconnected.flatMap(_.collectConstants(instances))

      Some(
        Project(
          gameName,
          instances,
          connected,
          constants,
          distanceMatrix,
          busDistanceMatrix,
          wires
        )
      )
    }

    private def flattenContainers(rootContainer: MutableContainer): Unit = {
      // flatten all containers
      for (c <- containerStack.popAll()) {
        rootContainer.mutableChildren ++= c.children
        rootContainer.mutableUnconnected ++= c.unconnected
      }

      containerStack.clear()
    }

    private def outputSoftware(
        rootContainer: Container,
        connected: Seq[Connected],
        distanceMatrix: DistanceMatrix
    ): Unit = {
      pushOutPath("soft/tmp")
      Utils.ensureDirectories(outPath)

      // get software (libraries, boot roms) for all cpus
      val softInstances = rootContainer.children.collect {
        case i: SoftwareInstance => i
      }
      val cpuInstances = rootContainer.children.collect { case i: CpuInstance =>
        i
      }
      // collect constants
      val constants =
        rootContainer.unconnected.flatMap(_.collectConstants(softInstances))

      OutputSoftware.cpuInvariantActions(softInstances, constants)
      OutputSoftware.cpuSpecificActions(
        board,
        softInstances,
        cpuInstances,
        distanceMatrix,
        connected
      )
      OutputSoftware.hardwareRegistersOutput(
        cpuInstances,
        distanceMatrix,
        connected
      )

      popOutPath()
    }

    private def generateInstances(path: Path): Boolean = {
      var newContainer = MutableContainer()
      containerStack.push(newContainer)
      val parsed = Utils.readYaml(path)
      if (parsed.contains("defaults"))
        defaults ++= Utils.toTable(parsed("defaults"))

      if (!processInstantiations(parsed, newContainer).getOrElse(false)) {
        // Handle the error appropriately
        println(s"Instantiation failed")
        false
      } else true
    }

    private def injectBoard(rootContainer: MutableContainer) = {
      // pass the board as if it had been a prefab in the main project file
      val boardInsertV = Map[String, Variant](
        "instance" -> ArrayV(
          Array(
            TableV(
              Map[String, Variant](
                "name" -> StringV(s"$board"),
                "type" -> StringV(s"board.$board")
              )
            )
          )
        ),
        "prefab" -> ArrayV(
          Array(
            TableV(
              Map[String, Variant]("name" -> StringV(s"boards.$board"))
            )
          )
        )
      )
      processInstantiations(boardInsertV, rootContainer)
    }

    private def connectAndOutputChips(rootContainer: MutableContainer) = {
      pushOutPath("gate")
      Utils.ensureDirectories(outPath)

      val chipInstances = rootContainer.children.collect {
        case i: ChipInstance => i
      }

      // collect constants
      val constants =
        rootContainer.unconnected.flatMap(_.collectConstants(chipInstances))

      // preconnect for bus address allocations
      rootContainer.unconnected.foreach(_.preConnect(chipInstances))

      // run gateware actions
      OutputGateware(rootContainer, outPath, constants, 1)
      OutputGateware(rootContainer, outPath, constants, 2)

      rootContainer.unconnected.foreach(_.finaliseBuses(chipInstances))

      // connect chips and uniquise the connections
      val connected =
        rootContainer.unconnected.flatMap(_.connect(chipInstances)).toSet.toSeq

      // compute the distance matrix
      val allDistanceMatrix: DistanceMatrix =
        DistanceMatrix(chipInstances, connected)
      val busDistanceMatrix: DistanceMatrix = DistanceMatrix(
        chipInstances,
        connected.filterNot { i =>
          i.isInstanceOf[ConnectedPortGroup]
        }
      )
      val wires = Wires(allDistanceMatrix, connected)
      popOutPath()

      (connected, allDistanceMatrix, busDistanceMatrix, wires)

    }

    private def processInstantiations(
        parsed: Map[String, Variant],
        container: MutableContainer
    ): Either[String, Boolean] = boundary {
      // includes
      if (parsed.contains("include")) {
        val tincs = Utils.toArray(parsed("include"))
        for (include <- tincs) {
          val table = Utils.toTable(include)
          val incResourceName = Utils.toString(table("resource"))
          val incResourcePath = Paths.get(incResourceName)

          prefabs.findPrefabInclude(incResourceName) match {
            case Some(value) =>
              Project.setInstancePath(value.path)
              val variants = prefabs.flattenIncludesContents(value.name)
              processInstantiations(variants, container) match {
                case Left(error) => break(Left(error))
                case _           => // continue
              }
              Project.popInstancePath()
            case None =>
              Utils.readFile(incResourcePath) match {
                case Some(_) =>
                  if (!generateInstances(incResourcePath)) {
                    break(Left("Failed to generate instances"))
                  }
                case _ =>
                  break(
                    Left(s"Include resource file $incResourceName not found")
                  )
              }
          }
        }
      }

      // extract instances
      if (parsed.contains("instance")) {
        val instancesWanted = Utils.toArray(parsed("instance"))
        val instances =
          instancesWanted.flatMap(v =>
            Instance(v, defaults.toMap, catalogs) match {
              case Right(instance) => Some(instance)
              case Left(error) =>
                println(s"Error creating instance: $error")
                None
            }
          )
        container.mutableChildren ++= instances
      }

      // extract connections
      if (parsed.contains("connection")) {
        val connections = Utils.toArray(parsed("connection"))
        container.mutableUnconnected ++= connections.flatMap(Unconnected(_))
      }

      // bring in wanted prefabs
      if (parsed.contains("prefab")) {
        val tpfs = Utils.toArray(parsed("prefab"))
        for (prefab <- tpfs) {
          val table = Utils.toTable(prefab)
          val ident = Utils.toString(table("name"))
          prefabs.findPrefab(ident) match {
            case Some(pf) =>
              processInstantiations(pf.stuff, container) match {
                case Left(error) => break(Left(error))
                case _           => // continue
              }
            case None =>
              break(Left(s"Error: prefab $ident not found"))
          }
        }
      }

      Right(true)
    }

    private def resolveSoftwareDependencies(
        rootContainer: MutableContainer
    ): Unit = {
      // add all instances, so we can check for software dependencies (including drivers for hardware)
      val depSet: mutable.Set[InstanceTrait] =
        rootContainer.children.to(mutable.Set)

      // push all there dependencies onto stack
      val depStack = mutable.Stack[String]()
      depSet.foreach(_.definition.dependencies.foreach(depStack.push))
      while (depStack.nonEmpty) {
        val n = depStack.pop()
        // if already exists ignore (and we can assume all children have already are/been on the stack
        if (
          !depSet.exists(_.definition.defType.ident.tail.mkString(".") == n)
        ) {
          val defs = {
            val si = n.split('.')
            if (si.isEmpty) Seq()
            else if (si(0) == "library") Seq(LibraryDefinitionType(si.toSeq))
            else if (si(0) == "program") Seq(ProgramDefinitionType(si.toSeq))
            else
              Seq(
                LibraryDefinitionType(si.prepended("library").toSeq),
                ProgramDefinitionType(si.prepended("program").toSeq)
              )
          }

          var found = false;

          for (
            d <- defs
            if !found
          ) {
            catalogs.findDefinition(d) match {
              case Some(defi) =>
                val result = defi.createInstance(n, Map[String, Variant]())
                result match {
                  case Right(inst) =>
                    assert(inst.isInstanceOf[SoftwareInstance])
                    rootContainer.mutableChildren ++= Seq(inst)
                    depSet += inst.asInstanceOf[SoftwareInstance]
                    inst.definition.dependencies
                      .filterNot(d => depStack.contains(d))
                      .foreach(depStack.push)
                    found = true
                  case Left(error) =>
                    println(
                      s"Failed to create instance for dependency $n: $error"
                    )
                    println(
                      s"ERROR: Software dependency $n ${defi.defType.ident.mkString(".")}"
                    )
                }
              case None =>
            }
          }
          if (!found) println(s"ERROR: Software dependency $n not found")
        }
      }
    }
  }

}
