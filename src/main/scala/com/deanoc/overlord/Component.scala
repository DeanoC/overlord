package com.deanoc.overlord

import com.deanoc.overlord.config.{InfoConfig, ComponentFileConfig}
import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.instances.MutableContainer
import java.nio.file.Path
import scala.util.boundary

/**
 * Represents a component in the Overlord system.
 * Components are Project objects with a required info section.
 *
 * @param name The name of the component
 * @param path The path to the component
 * @param config The configuration of the component
 * @param info The required info section for the component
 * @param container The parsed container with instances and connections
 * @param components A map of sub-components contained within this component
 */
case class Component(
  name: String,
  path: String,
  config: ComponentFileConfig,
  info: InfoConfig,
  container: MutableContainer,
  catalog: DefinitionCatalog,
  components: Map[String, Component] = Map.empty
) {
  /**
   * Gets the info section of this component.
   *
   * @return The info section
   */
  def getInfo: InfoConfig = info

  /**
   * Gets all sub-components of this component.
   *
   * @return A map of component names to components
   */
  def getComponents: Map[String, Component] = components

  /**
   * Gets a specific sub-component by name.
   *
   * @param name The name of the sub-component to retrieve
   * @return An Option containing the component if found, None otherwise
   */
  def getComponent(name: String): Option[Component] = components.get(name)

  /**
   * Checks if this component has a specific sub-component.
   *
   * @param name The name of the sub-component to check for
   * @return true if the sub-component exists, false otherwise
   */
  def hasComponent(name: String): Boolean = components.contains(name)

  /**
   * Gets the number of sub-components in this component.
   *
   * @return The number of sub-components
   */
  def componentCount: Int = components.size
  /**
   * Gets the container with instances and connections.
   *
   * @return The container
   */
  def getContainer: MutableContainer = container

  /**
   * Gets the definition catalog.
   *
   * @return The definition catalog
   */
  def getCatalog: DefinitionCatalog = catalog
}

/**
 * Companion object for Component class.
 */
object Component extends Logging {
  /**
   * Creates a Component from a project file.
   *
   * @param name The name of the component
   * @param board The board name
   * @param gamePath The path to the project file
   * @return An Option containing the Component if parsing was successful, None otherwise
   */
  def fromProjectFile(
      name: String,
      board: String,
      gamePath: Path
  ): Option[Component] = boundary {
    // Create a ProjectParser with explicit dependency injection
    val parser = new ComponentParser()

    // Parse the project file and get the container and catalog
    val (container, catalog) =
      parser.parseComponentFile(gamePath, board).getOrElse {
        this.error(s"Failed to parse project file: $gamePath")
        boundary.break(None)
      }
    
    // Create a Component from the parsed data
    val parsedConfig: Either[io.circe.Error, ComponentFileConfig] = for {
      yamlString <- Right(scala.io.Source.fromFile(gamePath.toFile).mkString)
      json <- io.circe.yaml.parser.parse(yamlString)
      config <- json.as[ComponentFileConfig]
    } yield config
    
    parsedConfig match {
      case Left(err) =>
        this.error(s"Failed to parse project file $gamePath: $err")
        boundary.break(None)
      case Right(config) =>
        Some(Component(
          name = name,
          path = gamePath.toString,
          config = config,
          info = config.info,
          container = container,
          catalog = catalog,
          components = Map.empty // Components will be populated in a future task
        ))
    }
  }
}