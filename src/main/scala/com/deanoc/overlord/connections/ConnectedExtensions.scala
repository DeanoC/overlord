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

    /** Gets the first full name of this connection */
    def firstFullName: String = connected.firstFullName

    /** Gets the second full name of this connection */
    def secondFullName: String = connected.secondFullName

    /** Gets the direction of this connection */
    def direction: ConnectionDirection = connected.direction

    /** Gets the priority of this connection */
    def priority: ConnectionPriority = connected.priority

    /** Checks if this connection is between the specified instances */
    def isBetween(first: InstanceTrait, second: InstanceTrait): Boolean =
      (connected.firstFullName == first.fullName && connected.secondFullName == second.fullName) ||
        (connected.firstFullName == second.fullName && connected.secondFullName == first.fullName)

    /** Checks if this connection is between the specified instance names */
    def isBetween(firstName: String, secondName: String): Boolean =
      (connected.firstFullName == firstName && connected.secondFullName == secondName) ||
        (connected.firstFullName == secondName && connected.secondFullName == firstName)

    /** Checks if this connection involves the specified instance */
    def involves(instance: InstanceTrait): Boolean =
      connected.firstFullName == instance.fullName || connected.secondFullName == instance.fullName

    /** Checks if this connection involves the specified instance name */
    def involves(instanceName: String): Boolean =
      connected.firstFullName == instanceName || connected.secondFullName == instanceName

    /** Gets the other instance name in this connection */
    def otherName(instanceName: String): Option[String] =
      if connected.firstFullName == instanceName then
        Some(connected.secondFullName)
      else if connected.secondFullName == instanceName then
        Some(connected.firstFullName)
      else None

    /** Gets the other instance in this connection */
    def otherInstance(instance: InstanceTrait): Option[InstanceTrait] =
      otherName(instance.fullName).flatMap(name =>
        instance.project.findInstance(name)
      )

  /** Extension methods for the ConnectedBetween trait */
  extension (connectedBetween: ConnectedBetween)
    /** Checks if this connection is between two chip instances */
    def isChipToChip(instances: Seq[ChipInstance]): Boolean =
      val firstChip =
        instances.find(_.fullName == connectedBetween.firstFullName)
      val secondChip =
        instances.find(_.fullName == connectedBetween.secondFullName)
      firstChip.isDefined && secondChip.isDefined
