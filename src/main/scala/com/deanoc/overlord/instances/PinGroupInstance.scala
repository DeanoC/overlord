package com.deanoc.overlord.instances

import com.deanoc.overlord.hardware.{BitsDesc, Port, WireDirection}
import com.deanoc.overlord.utils.Variant
import com.deanoc.overlord.{
  DiffPinConstraint,
  PinConstraint,
  PinConstraintType
}
import com.deanoc.overlord.definitions.ChipDefinitionTrait
import com.deanoc.overlord.utils.Utils

import scala.collection.mutable

case class PinGroupInstance(
    name: String,
    constraint: PinConstraintType,
    override val definition: ChipDefinitionTrait,
    config: com.deanoc.overlord.config.PinGroupConfig // Store the specific config
) extends ChipInstance {

  override lazy val ports: mutable.HashMap[String, Port] =
    mutable.HashMap.from(
      definition.ports ++ constraint.ports.map(p => p.name -> p)
    )
}

object PinGroupInstance {
  def apply(
      name: String, // Keep name as it's part of InstanceTrait
      definition: ChipDefinitionTrait,
      config: com.deanoc.overlord.config.PinGroupConfig // Accept PinGroupConfig
  ): Either[String, PinGroupInstance] = {
    try {
      // The PinGroupConfig only contains 'pins' and 'direction'.
      // Other attributes like standard, pullup, etc., are assumed to be
      // part of the definition's attributes or handled elsewhere.
      
      val standard = Utils.lookupString(definition.config.attributesAsVariant, "standard", "LVCMOS33")
      val pullup = Utils.lookupBoolean(definition.config.attributesAsVariant, "pullup", false) // Assuming a single pullup value for the group
  
      val pinNames = config.pins
      val pullups = List.fill(pinNames.length)(pullup)
  
      // Create ports with the correct WireDirection
      val wireDirection = WireDirection(config.direction)
      val ports = pinNames.map(name => Port(name, BitsDesc(1), wireDirection))
  
      // For the directions parameter, we need to use the string representation
      val directionStrings = List.fill(pinNames.length)(config.direction)
  
      val constraint = PinConstraint(
        pinNames,
        ports, // Use the created ports
        standard,
        pinNames, // Assuming constraintPinNames are the same as pinNames
        directionStrings, // Use the direction strings
        pullups
      )
  
      Right(new PinGroupInstance(name, constraint, definition, config)) // Pass the config
    } catch {
      case e: Exception =>
        Left(s"Error creating PinGroupInstance: ${e.getMessage}")
    }
  }
}
