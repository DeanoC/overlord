package com.deanoc.overlord.Connections

import com.deanoc.overlord.Instances.InstanceTrait

/** Represents a constant connection between an instance and a parameter.
  *
  * This case class is used to define a constant value or configuration
  * parameter associated with a specific instance in the system.
  *
  * @param instance
  *   The instance to which the constant is connected.
  * @param parameter
  *   The parameter representing the constant value or configuration.
  */
case class Constant(instance: InstanceTrait, parameter: Parameter) {}
