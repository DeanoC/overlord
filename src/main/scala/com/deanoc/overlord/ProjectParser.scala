package com.deanoc.overlord

import com.deanoc.overlord.connections._
import com.deanoc.overlord.instances._
import com.deanoc.overlord.utils.{
  Utils,
  Variant,
  ArrayV,
  TableV,
  StringV,
  Logging
}

import java.nio.file.{Path, Paths}
import scala.collection.mutable
import scala.util.boundary, boundary.break

/** Handles parsing of project files for the Overlord system. Responsible for
  * processing instantiations, includes, prefabs, and connections.
  */
class ProjectParser(
    catalogs: DefinitionCatalog,
    prefabs: PrefabCatalog
) extends Logging {
  private val containerStack: mutable.Stack[Container] = mutable.Stack()
  private val defaults = mutable.Map[String, Variant]()

  /** Generates instances from a project file path.
    *
    * @param path
    *   The path to the project file.
    * @return
    *   The container with generated instances, or None if generation fails.
    */
  def generateInstances(path: Path): Option[MutableContainer] = {
    val newContainer = MutableContainer()
    containerStack.push(newContainer)
    val parsed = Utils.readYaml(path)

    if (parsed.contains("defaults"))
      defaults ++= Utils.toTable(parsed("defaults"))

    if (!processInstantiations(parsed, newContainer).getOrElse(false)) {
      error(s"Instantiation failed")
      None
    } else {
      Some(newContainer)
    }
  }

  /** Processes instantiations from parsed YAML data.
    *
    * @param parsed
    *   The parsed YAML data as a map.
    * @param container
    *   The container to which instances should be added.
    * @return
    *   Either a success value or an error message.
    */
  def processInstantiations(
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

        debug(s"include: $incResourceName at $incResourcePath")

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
                generateInstances(incResourcePath) match {
                  case Some(_) => // Success
                  case None    => break(Left("Failed to generate instances"))
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
            case Left(err) =>
              error(s"creating instance: $err")
              None
          }
        )
      container.mutableChildren ++= instances
    }

    // extract connections
    if (parsed.contains("connection")) {
      val connections = Utils.toArray(parsed("connection"))
      container.mutableUnconnected ++= connections.flatMap(
        ConnectionParser.parseConnection(_)
      )
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
              case Left(err) => break(Left(err))
              case _         => // continue
            }
          case None =>
            break(Left(s"prefab $ident not found"))
        }
      }
    }

    Right(true)
  }

  /** Gets all containers from the stack.
    *
    * @return
    *   All containers in the stack.
    */
  def getAllContainers: Seq[Container] = containerStack.toSeq

  /** Clears the container stack.
    */
  def clearContainers(): Unit = containerStack.clear()
}
