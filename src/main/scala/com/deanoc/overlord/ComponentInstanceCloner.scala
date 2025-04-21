package com.deanoc.overlord

import com.deanoc.overlord.instances.{InstanceTrait, MutableContainer}
import com.deanoc.overlord.connections.Connected
import com.deanoc.overlord.utils.{Logging, NamespaceUtils}
import com.deanoc.overlord.config.{InstanceConfig, ComponentFileConfig, ConnectionConfig}

/**
 * Utility class for cloning instances and connections.
 * This is used when instantiating components to create copies of the component's
 * instances and connections with proper namespacing.
 */
class ComponentInstanceCloner extends Logging {
  
  /**
   * Clones all instances from a component with proper namespacing.
   *
   * @param namespace The namespace to apply to the cloned instances
   * @param component The component containing the instances to clone
   * @param instanceConfig Optional configuration to apply to the cloned instances
   * @return A sequence of cloned instances with proper namespacing
   */
  def cloneInstances(
    namespace: String,
    component: Component,
    instanceConfig: Map[String, Any]
  ): Seq[InstanceTrait] = {
    // Get all instances from the component
    val instances = component.getContainer.children
    
    // Clone each instance with proper namespacing
    instances.map { instance =>
      // Apply namespace to the instance
      val namespacedInstance = NamespaceUtils.namespaceInstance(namespace, instance)
      
      warn(s"Applying configuration to instance ${namespacedInstance.name} not yet implemented")
      namespacedInstance
    }
  }
  
  /**
   * Clones all connections from a component with proper namespacing.
   *
   * @param namespace The namespace to apply to the cloned connections
   * @param component The component containing the connections to clone
   * @return A sequence of cloned connections with proper namespacing
   */
  def cloneConnections(
    namespace: String,
    component: Component
  ): Seq[ConnectionConfig] = {
    // Get all connections from the component
    val connections = component.config.connections
    
    // Clone each connection with proper namespacing
    connections.map { connection =>
      NamespaceUtils.namespaceConnection(namespace, connection)
    }
  }
  
  /**
   * Clones a component's instances and connections into a container.
   *
   * @param namespace The namespace to apply to the cloned instances and connections
   * @param component The component to clone
   * @param container The container to add the cloned instances and connections to
   * @param instanceConfig Optional configuration to apply to the cloned instances
   * @return The container with the cloned instances and connections added
   */
  def cloneComponentIntoContainer(
    namespace: String,
    component: Component,
    container: MutableContainer,
    instanceConfig: Map[String, Any]
  ): MutableContainer = {
    // Clone instances with proper namespacing
    val clonedInstances = cloneInstances(namespace, component, instanceConfig)
    
    // Clone connections with proper namespacing
    val clonedConnections = cloneConnections(namespace, component)
    
    // Add cloned instances to the container
    container.mutableChildren ++= clonedInstances
    
    // Add cloned connections to the container
    // Note: This is a simplified implementation that assumes we can just add the connections to the container.
    // In a real implementation, we would need to handle different connection types differently.
    // container.mutableUnconnected ++= clonedConnections
    
    // Return the container with the cloned instances and connections
    container
  }
  
  /**
   * Creates a new container with cloned instances and connections from a component.
   *
   * @param namespace The namespace to apply to the cloned instances and connections
   * @param component The component to clone
   * @param instanceConfig Optional configuration to apply to the cloned instances
   * @return A new container with the cloned instances and connections
   */
  def cloneComponentToNewContainer(
    namespace: String,
    component: Component,
    instanceConfig: Map[String, Any]
  ): MutableContainer = {
    // Create a new container
    val container = new MutableContainer()
    
    // Clone the component into the container
    cloneComponentIntoContainer(namespace, component, container, instanceConfig)
  }
}

/**
 * Companion object for ComponentInstanceCloner
 */
object ComponentInstanceCloner {
  /**
   * Creates a new ComponentInstanceCloner
   *
   * @return A new ComponentInstanceCloner
   */
  def apply(): ComponentInstanceCloner = new ComponentInstanceCloner()
}