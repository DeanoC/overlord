package com.deanoc.overlord

import com.deanoc.overlord.connections._
import com.deanoc.overlord.instances._
import com.deanoc.overlord.interfaces.{ChipLike, RegisterBankLike}
import com.deanoc.overlord.utils.{
  Utils,
  Variant,
  ArrayV,
  TableV,
  StringV,
  Logging
}
import com.deanoc.overlord.definitions.{GatewareDefinitionTrait, SoftwareDefinitionTrait}
import com.deanoc.overlord.definitions.DefinitionType

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable
import scala.reflect.ClassTag
import scala.util.boundary, boundary.break

/** Represents a project in the Overlord system, containing instances,
  * connections, constants, and more. Provides utility methods for managing and
  * interacting with the project structure.
  */
case class Overlord(
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

object Overlord extends Logging {
  // Stacks for managing paths during project setup and execution.
  private val catalogPathStack: mutable.Stack[Path] = mutable.Stack[Path]()
  private val instancePathStack: mutable.Stack[Path] = mutable.Stack[Path]()
  private val outPathStack: mutable.Stack[Path] = mutable.Stack[Path]()

  private var baseProjectPath: Path = Paths.get("")

  /** For testing purposes only: Get the current catalog path stack.
    *
    * @return
    *   The current catalog path stack
    */
  def getCatalogPathStackForTesting: mutable.Stack[Path] = catalogPathStack

  def resetPaths(): Unit = {
    catalogPathStack.clear()
    instancePathStack.clear()
    outPathStack.clear()
  }

  /** Sets up the paths for the project, including catalog, prefab, and output
    * paths.
    *
    * @param projectPath
    *   The base path of the project.
    * @param catalogPath
    *   The path to the catalog directory.
    * @param prefabsPath
    *   The path to the prefabs directory.
    */
  def setupPaths(
      projectPath: Path
  ): Unit = {
    baseProjectPath = projectPath.toAbsolutePath

    // Use the project file's directory as the output location
    val projectDir =
      if (projectPath.toFile.isFile) projectPath.getParent else projectPath

    catalogPathStack.clear()

    instancePathStack.clear()

    outPathStack.clear()
    outPathStack.push(projectDir)
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
    val potDPath =
      if (catalogPathStack.isEmpty) path
      else catalogPath.resolve(path)

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
    * @return
    *   An optional Project instance.
    */
  def apply(
      gameName: String,
      board: String,
      gamePath: Path
  ): Overlord = boundary {
    // Create a Component from the project file
    val component = Component.fromProjectFile(gameName, board, gamePath).getOrElse {
      this.error(s"Failed to create component from project file: $gamePath")
      boundary.break(null)
    }

    // Get the container and catalog from the component
    val container = component.getContainer
    val catalog = component.getCatalog

    // Resolve software dependencies with explicit dependency injection
    resolveSoftwareDependencies(container, catalog)

    // Connect everything with explicit dependency injection
    val (connected, distanceMatrix, busDistanceMatrix, wires) =
      connectAndOutputChips(container)
    
    // Output software with explicit dependency injection
    outputSoftware(container, board, connected, busDistanceMatrix)

    // Get chips (hardware or gateware)
    val instances = container.children.collect {
      case i: ChipInstance     => i
      case s: SoftwareInstance => s
    }

    // Collect constants with explicit dependency injection
    val constants =
      container.unconnected.flatMap(_.collectConstants(instances))

    // Create the Overlord instance with explicit dependency injection
    Overlord(
      gameName,
      instances,
      connected,
      constants,
      distanceMatrix,
      busDistanceMatrix,
      wires
    )
  }

  private def flattenContainers(
      rootContainer: MutableContainer,
      containers: Seq[Container]
  ): Unit = {
    // flatten all containers
    for (c <- containers) {
      rootContainer.mutableChildren ++= c.children
      rootContainer.mutableUnconnected ++= c.unconnected
    }
  }

  private def outputSoftware(
      rootContainer: Container,
      board: String,
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

  private def connectAndOutputChips(rootContainer: MutableContainer) = {
    pushOutPath("gate")
    Utils.ensureDirectories(outPath)

    val chipInstances = rootContainer.children.collect { case i: ChipInstance =>
      i
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

  private def resolveSoftwareDependencies(
      rootContainer: MutableContainer,
      catalogs: DefinitionCatalog
  ): Unit = {
    // Add all instances, so we can check for software dependencies (including drivers for hardware)
    val depSet: mutable.Set[InstanceTrait] =
      rootContainer.children.to(mutable.Set)

    // Push all dependencies onto stack
    val depStack = mutable.Stack[String]()
    depSet.foreach(_.definition.dependencies.foreach(depStack.push))
    
    while (depStack.nonEmpty) {
      val n = depStack.pop()
      // If already exists ignore (and we can assume all children have already are/been on the stack)
      if (!depSet.exists(_.definition.defType.ident.tail.mkString(".") == n)) {
        val defs = {
          val si = n.split('.')
          if (si.isEmpty) Seq()
          else if (si(0) == "library") Seq(DefinitionType.LibraryDefinition(si.toSeq))
          else if (si(0) == "program") Seq(DefinitionType.ProgramDefinition(si.toSeq))
          else
            Seq(
              DefinitionType.LibraryDefinition(si.prepended("library").toSeq),
              DefinitionType.ProgramDefinition(si.prepended("program").toSeq)
            )
        }

        var found = false;

        for (
          d <- defs
          if !found
        ) {
          catalogs.findDefinition(d) match {
            case Some(defi) =>
              // Create appropriate configuration based on definition type
              val config = createConfigForDefinition(defi.defType, n)
              
              // Create instance with explicit configuration injection
              val result = defi.createInstance(n, Some(config))
              result match {
                case Right(inst) =>
                  assert(inst.isInstanceOf[SoftwareInstance])
                  rootContainer.mutableChildren ++= Seq(inst)
                  depSet += inst.asInstanceOf[SoftwareInstance]
                  inst.definition.dependencies
                    .filterNot(d => depStack.contains(d))
                    .foreach(depStack.push)
                  found = true
                case Left(errorMessage) =>
                  this.error(s"Failed to create instance for software dependency $n: $errorMessage in ${defi.defType.ident.mkString(".")}")
              }
            case None =>
          }
        }
        if (!found) this.error(s"Software dependency $n not found")
      }
    }
  }
  
  // Helper method to create appropriate configuration based on definition type
  private def createConfigForDefinition(defType: DefinitionType, name: String): Map[String, Any] = {
    defType match {
      case _: DefinitionType.LibraryDefinition =>
        // Create LibraryConfig with empty dependencies
        Map("dependencies" -> List.empty[String])
      case _: DefinitionType.ProgramDefinition =>
        // Create ProgramConfig with empty dependencies
        Map("dependencies" -> List.empty[String])
      case _ =>
        // Default empty config
        Map.empty[String, Any]
    }
  }

  private def tryPaths(
      instance: InstanceTrait,
      resource: String,
      pass: Int
  ): Path = {
    val givenPath = Paths.get(resolvePathMacros(instance, resource))
    if (!Files.exists(givenPath)) {
      val instancePath = Paths
        .get(
          Overlord.resolvePathMacros(instance, "${instancePath}/" + resource)
        )
        .normalize()
      if (!Files.exists(instancePath)) {
        val definitionPath = Paths
          .get(
            Overlord
              .resolvePathMacros(instance, "${definitionPath}/" + resource)
          )
          .normalize()
        if (!Files.exists(definitionPath)) {
          val outPath = Paths
            .get(Overlord.resolvePathMacros(instance, "${outPath}/" + resource))
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
}
