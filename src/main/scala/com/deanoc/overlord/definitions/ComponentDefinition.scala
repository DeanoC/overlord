package com.deanoc.overlord.definitions

import com.deanoc.overlord.Component
import com.deanoc.overlord.instances.{InstanceTrait}
import com.deanoc.overlord.connections.Connected
import java.nio.file.Path
import com.deanoc.overlord.config.DefinitionConfig

/**
 * Represents a component definition in the Overlord framework.
 * A component is a reusable collection of instances and connections that can be instantiated in a project.
 *
 * @param name The name of the component
 * @param component The Component object containing the component's instances and connections
 * @param sourcePath The path to the component's source file
 */
class ComponentDefinition(
  val name: String,
  val config: DefinitionConfig,
  val component: Component,
  val sourcePath: Path
) extends DefinitionTrait {
  
  /**
   * The type of this definition
   */
  override val defType: DefinitionType = DefinitionType.ComponentDefinition(name.split('.').toSeq)
  
  /**
   * The attributes of this definition
   */

  
  /**
   * The dependencies of this definition
   */
  override val dependencies: Seq[String] = Seq.empty
  
  /**
   * Creates an instance of this component.
   * This method is responsible for cloning instances and connections from the component's parsed data.
   * 
   * Note: In future phases, this method will implement the full cloning logic including namespacing.
   * Currently, it provides a basic implementation acknowledging its future role.
   *
   * @param name The name to give to the new instance
   * @param instanceConfig Optional configuration for the instance
   * @return A new instance created from this definition
   */
  override def createInstance(
    name: String,
    instanceConfig: Map[String, Any]
  ): Either[String, InstanceTrait] = {
    try {
      // Create a ComponentInstanceCloner to handle cloning
      val cloner = new com.deanoc.overlord.ComponentInstanceCloner()
      
      // Create a new container to hold the cloned instances and connections
      val container = cloner.cloneComponentToNewContainer(name, component, instanceConfig)
      
      // Return the container as the component instance
      // Since we can't create a Container directly (it's a trait),
      // we'll return the MutableContainer we created
      
      // For now, we'll return a placeholder error since we need to create a proper instance
      // that implements InstanceTrait, not just Container
      Left("Component instantiation not yet fully implemented - container created but cannot be returned as InstanceTrait")
    } catch {
      case e: Exception =>
        Left(s"Failed to create component instance: ${e.getMessage}")
    }
  }
  
  /**
   * Gets the information section of the component.
   *
   * @return The information from the component
   */
  def getInfo = component.getInfo
  
  /**
   * Gets all instances defined in this component.
   *
   * @return A sequence of instances
   */
  def getInstances: Seq[InstanceTrait] = component.getContainer.children
  
  /**
   * Gets all connections defined in this component.
   *
   * @return A sequence of connections
   */
  def getConnections: Seq[Connected] = {
    // In the current implementation, we don't have direct access to connected instances
    // This is a placeholder that will be implemented in future phases
    // when the connection logic is fully implemented
    Seq.empty[Connected]
  }
}

/**
 * Companion object for ComponentDefinition
 */
object ComponentDefinition {
  /**
   * Creates a ComponentDefinition from a Component
   *
   * @param component The Component to create a definition from
   * @return A new ComponentDefinition
   */
  def apply(component: Component): ComponentDefinition = {
    new ComponentDefinition(
      name = component.name,
      config = component.config.asInstanceOf[DefinitionConfig],
      component = component,
      sourcePath = java.nio.file.Paths.get(component.path)
    )
  }
}