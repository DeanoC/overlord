package com.deanoc.overlord.connections

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.instances.{HardwareInstance, InstanceTrait}
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord._
import com.deanoc.overlord.interfaces._
import com.deanoc.overlord.connections.ConnectionParser

/**
  * Represents the type of a parameter.
  */
sealed trait ParameterType

/**
  * Represents a constant parameter type with a specific value.
  *
  * @param value
  *   The value of the constant parameter.
  */
case class ConstantParameterType(value: Variant) extends ParameterType

/**
  * Represents a frequency parameter type with a specific frequency.
  *
  * @param freq
  *   The frequency value of the parameter.
  */
case class FrequencyParameterType(freq: Double) extends ParameterType

/**
  * Represents a parameter with a name and a specific type.
  *
  * @param name
  *   The name of the parameter.
  * @param parameterType
  *   The type of the parameter.
  */
case class Parameter(name: String, parameterType: ParameterType)

/**
  * Represents unconnected parameters for a specific instance.
  *
  * This case class defines the properties and methods for managing
  * unconnected parameters, including collecting constants and establishing
  * connections.
  *
  * @param direction
  *   The direction of the connection.
  * @param instanceName
  *   The name of the instance associated with the parameters.
  * @param parameters
  *   A sequence of parameters to be connected.
  */
case class UnconnectedParameters(
    direction: ConnectionDirection,
    instanceName: String,
    parameters: Seq[Parameter]
) extends Unconnected {

  /**
    * Establishes the connection for the parameters (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    * @return
    *   An empty sequence of connected components.
    */
  override def connect(unexpanded: Seq[HardwareInstance]): Seq[Connected] = Seq()

  /**
    * Collects constants associated with the unconnected parameters.
    *
    * @param unexpanded
    *   A sequence of unexpanded instance traits.
    * @return
    *   A sequence of constants for the parameters.
    */
  override def collectConstants(
      unexpanded: Seq[InstanceTrait]
  ): Seq[Constant] = {
    val instances = matchInstances(instanceName, unexpanded)
    if (instances.isEmpty) return Seq()

    // constants hookup before gateware has generated its ports, so we assume that the port specified is valid
    for {
      instance <- instances
      param <- parameters
    } yield Constant(instance.instance, param)
  }

  /**
    * Performs pre-connection checks for the parameters (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def preConnect(unexpanded: Seq[HardwareInstance]): Unit = None

  /**
    * Finalizes the parameter connections (no operation for this implementation).
    *
    * @param unexpanded
    *   A sequence of unexpanded chip instances.
    */
  override def finaliseBuses(unexpanded: Seq[HardwareInstance]): Unit = None
}

/**
  * Factory object for creating instances of `UnconnectedParameters`.
  */
object UnconnectedParameters {

  /**
    * Creates an `UnconnectedParameters` instance from a direction, instance name, and parameter variants.
    *
    * @param direction
    *   The direction of the connection.
    * @param secondFullName
    *   The name of the instance associated with the parameters.
    * @param parametersV
    *   An array of parameter variants.
    * @return
    *   An instance of `UnconnectedParameters`.
    */
  def apply(
      direction: ConnectionDirection,
      secondFullName: String,
      parametersV: Array[Variant]
  ): UnconnectedLike = {
    // Delegate to the ConnectionParser for parsing parameter connections
    ConnectionParser.parseParametersConnection(direction, secondFullName, parametersV)
  }
}
