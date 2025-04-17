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
import com.deanoc.overlord.connections.ConnectionTypes.BusName.default

/** Handles parsing of project files for the Overlord system. Responsible for
  * processing instantiations, includes, prefabs, and connections.
  */
class ProjectParser() extends Logging {
  private val containerStack: mutable.Stack[Container] = mutable.Stack()

  def parseProjectFile(
      path: Path,
      board: String
  ): Option[(MutableContainer, DefinitionCatalog)] = {
    val newContainer = MutableContainer()
    containerStack.push(newContainer)

    val parsed = Utils.readYaml(path) ++ insertBoard(board)
    val boards = parseBoards(parsed)
    if (!boards.contains(board)) {
      error(
        s"The board $board is not supported by this project. The boards supported are $boards"
      )
      return None
    }
    val defaults = parseDefaults(parsed)
    val catalogs = parseCatalogs(parsed, defaults)
    val prefabs = parsePrefabs(parsed)

    processInstantiations(
      parsed,
      catalogs,
      prefabs,
      defaults,
      newContainer
    ) match {
      case Left(err) =>
        error(s"Error processing instantiations: $err")
        None
      case Right(project) =>
        Some((newContainer, catalogs))
    }
  }

  def parseCatalogs(
      parsed: Map[String, Variant],
      defaults: Map[String, Variant]
  ): DefinitionCatalog = {
    var catalogs: DefinitionCatalog = DefinitionCatalog()

    // Process catalogs first before any other operations
    if (parsed.contains("catalogs")) {
      val catalogsArray = Utils.toArray(parsed("catalogs"))

      val result = (for (catalog <- catalogsArray) yield {
        val name = Utils.toString(catalog)
        if (name.startsWith("https://") && name.endsWith(".git")) {
          DefinitionCatalog.fromURL(name, defaults)
        } else {
          DefinitionCatalog.fromFile(name, defaults)
        }
      }).flatten.flatten.map(f => f.defType -> f).toMap

      catalogs.mergeNewDefinition(result)
    }
    catalogs
  }

  def parsePrefabs(
      parsed: Map[String, Variant]
  ): PrefabCatalog = {
    // Process prefab paths defined in the project file
    if (parsed.contains("prefabs")) {
      val prefabCatalog =
        for (prefab <- Utils.toArray(parsed("prefabs"))) yield {
          val name = Utils.toString(prefab)
          if (name.startsWith("https://") && name.endsWith(".git")) {
            PrefabCatalog.fromURL(name)
          } else {
            PrefabCatalog.fromFile(name)
          }
        }

      prefabCatalog.foldLeft(PrefabCatalog())((acc, catalog) =>
        PrefabCatalog(acc.prefabs ++ catalog.prefabs)
      )
    } else PrefabCatalog()
  }

  def parseDefaults(parsed: Map[String, Variant]): Map[String, Variant] = {

    if (parsed.contains("defaults"))
      Utils.toTable(parsed("defaults"))
    else Map()
  }

  def insertBoard(board: String): Map[String, Variant] = {
    // pass the board as if it had been a prefab in the main project file
    Map[String, Variant](
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
  }

  def parseBoards(parsed: Map[String, Variant]): Seq[String] = {
    if (parsed.contains("boards")) {
      Utils.toArray(parsed("boards")).toIndexedSeq.map(Utils.toString)
    } else Seq.empty
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
      catalog: DefinitionCatalog,
      prefabs: PrefabCatalog,
      defaults: Map[String, Variant],
      container: MutableContainer
  ): Either[String, Boolean] = boundary {
    // extract instances
    if (parsed.contains("instance")) {
      val instancesWanted = Utils.toArray(parsed("instance"))
      val instances =
        instancesWanted.flatMap(v =>
          Instance(v, defaults.toMap, catalog) match {
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
            processInstantiations(
              pf.stuff,
              catalog,
              prefabs,
              defaults,
              container
            ) match {
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
