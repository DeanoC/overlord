package com.deanoc.overlord

import com.deanoc.overlord.connections._
import com.deanoc.overlord.instances._
import com.deanoc.overlord.utils.{Logging, Utils, NamespaceUtils}
import com.deanoc.overlord.config.{ComponentFileConfig, InstanceConfig, PrefabConfig, ConnectionConfig, InfoConfig, SourceConfig} // Import specific config case classes
import com.deanoc.overlord.definitions.{DefinitionType, ComponentDefinition}

import java.nio.file.Path
import scala.collection.mutable
import scala.util.boundary, boundary.break
import io.circe.yaml.parser
import io.circe.Decoder // Import Decoder explicitly

/** Handles parsing of project files for the Overlord system. Responsible for
  * processing instantiations, includes, prefabs, and connections.
  */
class ComponentParser() extends Logging {
  private val containerStack: mutable.Stack[MutableContainer] = mutable.Stack()

  def parseComponentFile(
      path: Path,
      board: String = ""
  ): Option[(MutableContainer, DefinitionCatalog)] = {
    val newContainer = MutableContainer()
    containerStack.push(newContainer)

    val parsedConfig: Either[io.circe.Error, ComponentFileConfig] = for {
      yamlString <- Right(scala.io.Source.fromFile(path.toFile).mkString)
      json <- parser.parse(yamlString)
      config <- json.as[ComponentFileConfig]
    } yield config

    parsedConfig match {
      case Left(err) =>
        error(s"Failed to parse project file $path: $err")
        None
      case Right(parsed) =>
        // Initialize the definition catalog
        var definitionCatalog = new DefinitionCatalog
        
        // Process catalogs section
        if (parsed.catalogs.nonEmpty) {
          info(s"Processing ${parsed.catalogs.size} catalogs")
          // TODO: Implement catalog processing
          // This will be implemented in a future task
        }
        
        // Process components section
        if (parsed.components.nonEmpty) {
          info(s"Processing ${parsed.components.size} components")
          parsed.components.foreach { componentConfig =>
            processComponent(componentConfig, definitionCatalog, parsed.defaults)
          }
        }
        
        if (board.nonEmpty) {
          // TODO: new board selection mechanism
          val mergedParsed = parsed.copy(
            instances = parsed.instances
            // ++ List(InstanceConfig(name = board, `type` = s"board.$board")),
          )

          // Process prefabs using the new type-safe configuration
          val prefabCatalogResult = CatalogLoader.parsePrefabCatalog("project", mergedParsed, mergedParsed.defaults)
          
          val prefabCatalog = prefabCatalogResult match {
            case Left(err) =>
              error(s"Error processing prefabs: $err")
              PrefabCatalog()
            case Right(catalog) => catalog
          }

          newContainer.mutableChildren ++= processInstantiations(
            mergedParsed,
            definitionCatalog,
            prefabCatalog,
            parsed.defaults
          )
        } else {
          // Process instances directly without board-specific handling
          newContainer.mutableChildren ++= processInstantiations(
            parsed,
            definitionCatalog,
            PrefabCatalog(), // Empty prefab catalog when not using boards
            parsed.defaults
          )
        }
        
        Some((newContainer, definitionCatalog))
    }
  }
  
  /**
   * Processes a component configuration and registers it as a Component type definition.
   *
   * @param componentConfig The component configuration to process
   * @param catalog The definition catalog to register the component in
   * @param defaults The default values to use
   */
  private def processComponent(
    componentConfig: SourceConfig,
    catalog: DefinitionCatalog,
    defaults: Map[String, Any]
  ): Unit = {
    // Get the component path
    val componentPath = componentConfig.path match {
      case Some(path) => java.nio.file.Paths.get(path)
      case None =>
        error(s"Component ${componentConfig.`type`} has no path specified")
        return
    }
    
    // Load the component
    val componentName = componentConfig.`type`
    info(s"Loading component $componentName from $componentPath")
    
    // Create a Component from the project file
    val component = Component.fromProjectFile(componentName, "", componentPath) match {
      case Some(comp) => comp
      case None =>
        error(s"Failed to load component $componentName from $componentPath")
        return
    }
    
    // Register the component as a Component type definition
    catalog.addComponentDefinition(component)
    info(s"Registered component $componentName as a Component type definition")
  }


  // Updated to accept ProjectFileConfig
  def parseDefaults(parsed: ComponentFileConfig): Map[String, Any] = {
    parsed.defaults
  }

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
      parsed: ComponentFileConfig,
      catalog: DefinitionCatalog,
      prefabs: PrefabCatalog,
      defaults: Map[String, Any],
  ): Seq[InstanceTrait] = {
    // extract instances
    var instances = parsed.instances.flatMap { instanceConfig =>
        // Convert defaults to Variant map for backward compatibility
        val defaultsAsVariant = defaults.map { case (k, v) => k -> Utils.toVariant(v) }.toMap
        
        // Find the definition for this instance
        val defType = DefinitionType(instanceConfig.`type`)
        val definitionResult = catalog.findDefinition(defType) match {
          case Some(definition) => Right(definition)
          case None =>
            // Try to create a one shot definition if not found
            Instance.definitionFrom(catalog, Overlord.projectPath, instanceConfig, defType)
        }
        
        definitionResult match {
          case Right(definition) =>
            // Check if this is a Component type definition
            definition match {
              case componentDef: ComponentDefinition =>
                // This is a Component type definition, so we need to clone its instances and connections
                info(s"Creating instance ${instanceConfig.name} from component definition ${componentDef.name}")
                
                // Create a ComponentInstanceCloner to handle cloning
                val cloner = new ComponentInstanceCloner()
                
                // Try to create an instance from the component definition
                componentDef.createInstance(instanceConfig.name, instanceConfig.config) match {
                  case Right(inst) =>
                    // If successful, return the instance
                    Seq(inst)
                  case Left(errorMsg) =>
                    // If failed, log the error and fall back to cloning instances directly
                    this.warn(s"Failed to create component instance ${instanceConfig.name}: $errorMsg")
                    this.info(s"Falling back to direct instance cloning")
                    
                    // Clone the component's instances with proper namespacing
                    val clonedInstances = cloner.cloneInstances(
                      instanceConfig.name,
                      componentDef.component,
                      instanceConfig.config
                    )
                    
                    // Return the cloned instances
                    clonedInstances
                }
                
              case _ =>
                // This is a regular definition, so create the instance normally
                definition.createInstance(instanceConfig.name, instanceConfig.config) match {
                  case Right(inst) => Seq(inst)
                  case Left(errorMsg) =>
                    this.error(s"Failed to create instance ${instanceConfig.name}: $errorMsg", null)
                    Seq.empty
                }
            }
          case Left(errorMsg) =>
            this.error(s"Failed to find or create definition for ${instanceConfig.name}: $errorMsg", null)
            Seq.empty
        }
    }

    // Process prefabs (for backward compatibility)
    if (prefabs.prefabs.nonEmpty) {
      info(s"Processing ${prefabs.prefabs.size} prefabs")
      // TODO: Implement prefab processing
      // This will be implemented in a future task
    }

    instances
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
/* TODO prefab
    // Process prefabs using the new type-safe configuration
    parsed.prefabs.foreach { prefabConfig =>
        val ident = prefabConfig.name
        prefabs.findPrefab(ident) match {
          case Some(pf) =>
            // The prefab's configuration is already in the type-safe PrefabFileConfig format
            // Create a ProjectFileConfig from the PrefabFileConfig
            val prefabProjectConfig = ProjectFileConfig(
              instances = pf.config.instances.toList,
              connections = pf.config.connection.getOrElse(List.empty),
              defaults = defaults // Use the same defaults as the parent
            )
            
            // Process the prefab's instances and connections
            instances ++= processInstantiations(
              prefabProjectConfig,
              catalog,
              prefabs,
              defaults
            ) 
          case None =>
            error(s"Prefab $ident not found")
        }
      }
  }*/

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
