package com.deanoc.overlord

import com.deanoc.overlord.connections._
import com.deanoc.overlord.instances._
import com.deanoc.overlord.utils.{Logging, Utils, NamespaceUtils}
import com.deanoc.overlord.config.{ComponentFileConfig, InstanceConfig, ConnectionConfig, InfoConfig, SourceConfig} // Import specific config case classes
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

  def parseComponentFileConfig(parsed:ComponentFileConfig)
  : (MutableContainer, DefinitionCatalog) = {
    val newContainer = MutableContainer()
    containerStack.push(newContainer)

    var definitionCatalog = new DefinitionCatalog

    // Process the info section
    info(s"Processing info section: $parsed.info")
    processInfo(parsed.info)

    // Process the catalog parts of the file    
    definitionCatalog.mergeNewDefinition(
      CatalogLoader.processParsedCatalog(parsed)
    )

    // Process components section
    if (parsed.components.nonEmpty) {
      info(s"Processing ${parsed.components.size} components")
      parsed.components.foreach { componentConfig =>
        processComponent(componentConfig, definitionCatalog, parsed.defaults)
      }
    }
      
    // Process instances section
    if(parsed.instances.nonEmpty) {
      info(s"Processing ${parsed.instances.size} instances")
      // Process instances and connections
      newContainer.mutableChildren ++= processInstantiations(
        parsed,
        definitionCatalog,
        parsed.defaults
      )
    }

    // Process connections section
    if (parsed.connections.nonEmpty) {
      info(s"Processing ${parsed.connections.size} connections")
      processConnections(parsed.connections, newContainer, definitionCatalog)
    }
     
    (newContainer, definitionCatalog)
  }
  
  /**
   * Processes information from the info section of a component file.
   *
   * @param infoConfig The info configuration to process
   * @param container The container to update with info data
   */
  private def processInfo(
    infoConfig: InfoConfig
  ): Unit = {
    info(s"Processing info: ${infoConfig.name}")
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
    //val component = Component.fromComponentFile(componentName.toString(), "", componentPath)
    
    // Register the component as a Component type definition
    //catalog.addComponentDefinition(component)
    //info(s"Registered component $componentName as a Component type definition")
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
            // TODO: one shot definitions
            Left(s"No definition found or could be created (TODO: one shot definitions) for ${instanceConfig.name} $defType")

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
                componentDef.createInstance(instanceConfig.name, instanceConfig.attributes) match {
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
                      instanceConfig.attributes
                    )
                    
                    // Return the cloned instances
                    clonedInstances
                }
                
              case _ =>
                // This is a regular definition, so create the instance normally
                definition.createInstance(instanceConfig.name, instanceConfig.attributes) match {
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

    instances
  }

  /**
   * Processes connection configurations and adds them to the container.
   *
   * @param connections The list of connection configurations to process
   * @param container The container to which connections should be added
   * @param definitionCatalog The definition catalog for resolving references
   */
  private def processConnections(
    connections: List[ConnectionConfig],
    container: MutableContainer,
    definitionCatalog: DefinitionCatalog
  ): Unit = {
    // TODO: Implement connection processing logic
    info(s"Processing ${connections.size} connections")
    
    // For now, we'll just log the connection information
//    connections.foreach { connectionConfig =>
//     info(s"Connection: ${connectionConfig.from} -> ${connectionConfig.to}")
//    }
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
