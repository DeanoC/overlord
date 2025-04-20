package com.deanoc.overlord.utils

import com.deanoc.overlord.instances.InstanceTrait
import com.deanoc.overlord.connections.Connected
import com.deanoc.overlord.config.{ConnectionConfig, BusConnectionConfig, PortConnectionConfig, PortGroupConnectionConfig, ClockConnectionConfig, LogicalConnectionConfig, ParametersConnectionConfig, ConstantConnectionConfig}

/**
 * Utility class for handling namespacing of instances and connections.
 * This is used when instantiating components to ensure that instance names
 * and connection references are properly namespaced to avoid conflicts.
 */
object NamespaceUtils extends Logging {
  
  /**
   * Applies a namespace prefix to an instance name.
   * The format is "{namespace}.{name}" unless the name already has a namespace.
   *
   * @param namespace The namespace to apply
   * @param name The instance name
   * @return The namespaced instance name
   */
  def applyNamespace(namespace: String, name: String): String = {
    if (name.contains(".")) {
      // If the name already has a namespace, don't add another one
      name
    } else {
      s"$namespace.$name"
    }
  }
  
  /**
   * Updates a connection string with namespaced instance references.
   * For example, "instance1:port -> instance2:port" becomes "{namespace}.instance1:port -> {namespace}.instance2:port"
   *
   * @param namespace The namespace to apply
   * @param connectionString The connection string to update
   * @return The updated connection string with namespaced references
   */
  def namespaceConnectionString(namespace: String, connectionString: String): String = {
    // Split the connection string into parts (typically "from -> to" or similar)
    val parts = connectionString.split("->").map(_.trim)
    
    // Apply namespace to each part
    val namespacedParts = parts.map { part =>
      // Extract the instance name (everything before the first ":")
      val colonIndex = part.indexOf(":")
      if (colonIndex > 0) {
        val instanceName = part.substring(0, colonIndex).trim
        val portSpec = part.substring(colonIndex)
        
        // Apply namespace to the instance name
        val namespacedInstance = applyNamespace(namespace, instanceName)
        s"$namespacedInstance$portSpec"
      } else {
        // If there's no colon, assume the whole part is an instance name
        applyNamespace(namespace, part)
      }
    }
    
    // Rejoin the parts with the original separator
    namespacedParts.mkString(" -> ")
  }
  
  /**
   * Updates a connection configuration with namespaced instance references.
   *
   * @param namespace The namespace to apply
   * @param connection The connection configuration to update
   * @return The updated connection configuration with namespaced references
   */
  def namespaceConnection(namespace: String, connection: ConnectionConfig): ConnectionConfig = {
    // Update the connection string with namespaced references
    val namespacedConnectionString = namespaceConnectionString(namespace, connection.connection)
    
    // Create a new connection configuration with the updated connection string
    // Note: This is a simplified implementation that assumes we can just update the connection string.
    // In a real implementation, we would need to handle different connection types differently.
    connection match {
      case bus: BusConnectionConfig =>
        bus.copy(connection = namespacedConnectionString)
      case port: PortConnectionConfig =>
        port.copy(connection = namespacedConnectionString)
      case portGroup: PortGroupConnectionConfig =>
        portGroup.copy(connection = namespacedConnectionString)
      case clock: ClockConnectionConfig =>
        clock.copy(connection = namespacedConnectionString)
      case logical: LogicalConnectionConfig =>
        logical.copy(connection = namespacedConnectionString)
      case parameters: ParametersConnectionConfig =>
        parameters.copy(connection = namespacedConnectionString)
      case constant: ConstantConnectionConfig =>
        constant.copy(connection = namespacedConnectionString)
      case null =>
        // If we don't know how to handle this connection type, log a warning and return it unchanged
        warn(s"Unknown connection type: ${connection.getClass.getName}. Connection will not be namespaced.")
        connection
    }
  }
  
  /**
   * Applies a namespace to an instance by updating its name.
   *
   * @param namespace The namespace to apply
   * @param instance The instance to update
   * @return A new instance with the namespaced name
   */
  def namespaceInstance(namespace: String, instance: InstanceTrait): InstanceTrait = {
    // Apply namespace to the instance name
    val namespacedName = applyNamespace(namespace, instance.name)
    
    // In the current implementation, we can't directly create a new instance with a different name
    // This is a placeholder that will need to be implemented based on the actual instance types
    // For now, we'll log a warning and return the original instance
    warn(s"Namespacing instance ${instance.name} to $namespacedName not yet implemented")
    instance
  }
}