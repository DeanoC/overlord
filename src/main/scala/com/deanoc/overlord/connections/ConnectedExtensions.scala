package com.deanoc.overlord.connections

import com.deanoc.overlord.connections.ConnectionTypes._
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}

/** Extension methods for the Connected trait and related types.
  *
  * This object provides extension methods that enhance the functionality of the
  * Connected trait and related types, following Scala 3 best practices for
  * organizing code.
  */
object ConnectedExtensions:
  /** Extension methods for the Connected trait */
  extension (connected: Connected)
    /** Gets the name of this connection */
    def name: ConnectionName = connected.connectionName

    /** Gets the last part of the first name (after the last dot) */
    def firstLastName: String = 
      val parts = connected.firstFullName.split('.')
      parts.lastOption.getOrElse(connected.firstFullName)

    /** Gets the last part of the second name (after the last dot) */
    def secondLastName: String = 
      val parts = connected.secondFullName.split('.')
      parts.lastOption.getOrElse(connected.secondFullName)
    
    /** Gets the first part of the first name (before the first dot) */
    def firstHeadName: String = 
      val parts = connected.firstFullName.split('.')
      parts.headOption.getOrElse(connected.firstFullName)

    /** Gets the first part of the second name (before the first dot) */
    def secondHeadName: String = 
      val parts = connected.secondFullName.split('.')
      parts.headOption.getOrElse(connected.secondFullName)

    /** Checks if this connection is between the specified instances */
    def isBetween(first: InstanceTrait, second: InstanceTrait): Boolean =
      (connected.firstFullName.endsWith(first.name) && connected.secondFullName.endsWith(second.name)) ||
        (connected.firstFullName.endsWith(second.name) && connected.secondFullName.endsWith(first.name))

    /** Checks if this connection is between the specified instance names */
    def isBetween(firstName: String, secondName: String): Boolean =
      (connected.firstFullName.endsWith(firstName) && connected.secondFullName.endsWith(secondName)) ||
        (connected.firstFullName.endsWith(secondName) && connected.secondFullName.endsWith(firstName))

    /** Checks if this connection involves the specified instance */
    def involves(instance: InstanceTrait): Boolean =
      connected.firstFullName.endsWith(instance.name) || connected.secondFullName.endsWith(instance.name)

    /** Checks if this connection involves the specified instance name */
    def involves(instanceName: String): Boolean =
      connected.firstFullName.endsWith(instanceName) || connected.secondFullName.endsWith(instanceName)

    /** Gets the other instance name in this connection */
    def otherName(instanceName: String): Option[String] =
      if connected.firstFullName.endsWith(instanceName) then
        Some(connected.secondFullName)
      else if connected.secondFullName.endsWith(instanceName) then
        Some(connected.firstFullName)
      else None

  /** Extension methods for the ConnectedBetween trait */
  extension (connectedBetween: ConnectedBetween)
    /** Checks if this connection is between two chip instances */
    def isChipToChip(instances: Seq[ChipInstance]): Boolean =
      val firstChip =
        instances.find(chip => connectedBetween.firstFullName.endsWith(chip.name))
      val secondChip =
        instances.find(chip => connectedBetween.secondFullName.endsWith(chip.name))
      firstChip.isDefined && secondChip.isDefined
