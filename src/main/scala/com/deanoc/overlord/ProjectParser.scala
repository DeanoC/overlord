package com.deanoc.overlord

import com.deanoc.overlord.connections._
import com.deanoc.overlord.instances._
import com.deanoc.overlord.utils.{Logging, Utils}
import com.deanoc.overlord.config.{ProjectFileConfig, InstanceConfig, PrefabConfig, ConnectionConfig} // Import specific config case classes

import java.nio.file.Path
import scala.collection.mutable
import scala.util.boundary, boundary.break
import io.circe.yaml.parser
import io.circe.Decoder // Import Decoder explicitly

/** Handles parsing of project files for the Overlord system. Responsible for
  * processing instantiations, includes, prefabs, and connections.
  */
class ProjectParser() extends Logging {
  private val containerStack: mutable.Stack[MutableContainer] = mutable.Stack()

  def parseProjectFile(
      path: Path,
      board: String
  ): Option[(MutableContainer, DefinitionCatalog)] = {
    val newContainer = MutableContainer()
    containerStack.push(newContainer)

    val parsedConfig: Either[io.circe.Error, ProjectFileConfig] = for {
      yamlString <- Right(scala.io.Source.fromFile(path.toFile).mkString)
      json <- parser.parse(yamlString)
      config <- json.as[ProjectFileConfig]
    } yield config

    parsedConfig match {
      case Left(err) =>
        error(s"Failed to parse project file $path: $err")
        None
      case Right(parsed) =>
        val boards = parseBoards(parsed)
        if (!boards.contains(board)) {
          error(
            s"The board $board is not supported by this project. The boards supported are $boards"
          )
          return None
        }

        // Create a ProjectFileConfig for the board instance and merge it
        val boardConfig = ProjectFileConfig(
          instance = Some(List(InstanceConfig(name = board, `type` = s"board.$board"))),
          prefab = Some(List(PrefabConfig(name = s"boards.$board")))
        )

        // Simple merge: instances and prefabs from boardConfig are added to parsed
        val mergedParsed = parsed.copy(
          instance = (parsed.instance.getOrElse(List.empty) ++ boardConfig.instance.getOrElse(List.empty)).headOption.map(_ => (parsed.instance.getOrElse(List.empty) ++ boardConfig.instance.getOrElse(List.empty))),
          prefab = (parsed.prefab.getOrElse(List.empty) ++ boardConfig.prefab.getOrElse(List.empty)).headOption.map(_ => (parsed.prefab.getOrElse(List.empty) ++ boardConfig.prefab.getOrElse(List.empty)))
          // Note: This merge is basic and might need refinement based on how defaults and boards are intended to be merged.
        )


        var definitionCatalog = new DefinitionCatalog
        // TODO: Update CatalogLoader.parseDefinitionCatalog to accept ProjectFileConfig or relevant parts
        // For now, keeping the old call, which will need to be refactored later.
        // definitionCatalog.mergeNewDefinition(CatalogLoader.parseDefinitionCatalog("project", parsed, parsed.defaults.getOrElse(Map.empty).mapValues(Utils.toVariant)))

        // Process prefabs using the new type-safe configuration
        val prefabCatalogResult = CatalogLoader.parsePrefabCatalog(
          "project",
          mergedParsed,
          mergedParsed.defaults.getOrElse(Map.empty)
        )
        
        val prefabCatalog = prefabCatalogResult match {
          case Left(err) =>
            error(s"Error processing prefabs: $err")
            PrefabCatalog()
          case Right(catalog) => catalog
        }
        
        val catalogs = definitionCatalog
        val prefabs = prefabCatalog

        // TODO: Update processInstantiations to accept ProjectFileConfig
        // For now, passing the old Map[String, Variant] structure which needs to be removed
        processInstantiations(
          mergedParsed,
          catalogs,
          prefabs,
          parsed.defaults, // Pass Option[Map[String, Any]] directly
          newContainer
        ) match {
          case Left(err) =>
            error(s"Error processing instantiations: $err")
            None
          case Right(_) =>
            Some((newContainer, catalogs))
        }
    }
  }


  // Updated to accept ProjectFileConfig
  def parseDefaults(parsed: ProjectFileConfig): Option[Map[String, Any]] = {
    parsed.defaults
  }

  // This function is no longer needed as board insertion is handled in parseProjectFile
  // def insertBoard(board: String): Map[String, Variant] = {
  //   // pass the board as if it had been a prefab in the main project file
  //   Map[String, Variant](
  //     "instance" -> ArrayV(
  //       Array(
  //         TableV(
  //           Map[String, Variant](
  //             "name" -> StringV(s"$board"),
  //             "type" -> StringV(s"board.$board")
  //           )
  //         )
  //       )
  //     ),
  //     "prefab" -> ArrayV(
  //       Array(
  //         TableV(
  //           Map[String, Variant]("name" -> StringV(s"boards.$board"))
  //         )
  //       )
  //     )
  //   )
  // }

  // Updated to accept ProjectFileConfig
  def parseBoards(parsed: ProjectFileConfig): Seq[String] = {
    parsed.boards.getOrElse(List.empty)
  }

  /** Processes instantiations from parsed YAML data.
    *
    * @param parsed
    *   The parsed YAML data as a map. // TODO: Update to ProjectFileConfig
    * @param container
    *   The container to which instances should be added.
    * @return
    *   Either a success value or an error message.
    */
  /** Processes instantiations from parsed YAML data.
    *
    * @param parsed
    *   The parsed YAML data as a ProjectFileConfig.
    * @param container
    *   The container to which instances should be added.
    * @return
    *   Either a success value or an error message.
    */
  def processInstantiations(
      parsed: ProjectFileConfig,
      catalog: DefinitionCatalog,
      prefabs: PrefabCatalog,
      defaults: Option[Map[String, Any]],
      container: MutableContainer
  ): Either[String, Boolean] = boundary {
    // extract instances
    parsed.instance.foreach { instancesWanted =>
      val instances =
        instancesWanted.flatMap(instanceConfig => {
          // Convert defaults to Variant map for backward compatibility
          val defaultsAsVariant = defaults.getOrElse(Map.empty).map { case (k, v) => k -> Utils.toVariant(v) }.toMap
          
          // Find the definition for this instance
          val defType = DefinitionType(instanceConfig.`type`)
          val definitionResult = catalog.findDefinition(defType) match {
            case Some(definition) => Right(definition)
            case None =>
              // Try to create a definition if not found
              Instance.definitionFrom(catalog, Overlord.projectPath, instanceConfig, defType)
          }
          
          definitionResult match {
            case Right(definition) =>
              // Create the instance with the specific configuration
              definition.createInstance(instanceConfig.name, instanceConfig.config) match {
                case Right(inst) => Some(inst)
                case Left(errorMsg) =>
                  this.error(s"Failed to create instance ${instanceConfig.name}: $errorMsg", null)
                  None
              }
            case Left(errorMsg) =>
              this.error(s"Failed to find or create definition for ${instanceConfig.name}: $errorMsg", null)
              None
          }
        })
      container.mutableChildren ++= instances
    }

    // extract connections
    // TODO: Update ConnectionParser.parseConnection to accept ConnectionConfig
    // For now, we'll skip connection processing until ConnectionParser is updated
    // This is a temporary measure to allow compilation and testing of instance creation
    // container.mutableUnconnected ++= connections.flatMap(
    //   connectionConfig => {
    //     // This will be implemented in a future step
    //     None
    //   }
    // )

    // Process prefabs using the new type-safe configuration
    parsed.prefab.foreach { prefabsWanted =>
      for (prefabConfig <- prefabsWanted) {
        val ident = prefabConfig.name
        prefabs.findPrefab(ident) match {
          case Some(pf) =>
            // The prefab's configuration is already in the type-safe PrefabFileConfig format
            // Create a ProjectFileConfig from the PrefabFileConfig
            val prefabProjectConfig = ProjectFileConfig(
              instance = pf.config.instance,
              connections = pf.config.connection,
              defaults = defaults // Use the same defaults as the parent
            )
            
            // Process the prefab's instances and connections
            processInstantiations(
              prefabProjectConfig,
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
