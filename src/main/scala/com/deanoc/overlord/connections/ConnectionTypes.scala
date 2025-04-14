package com.deanoc.overlord.connections

import scala.annotation.targetName
import scala.util.matching.Regex

/** Defines opaque types for domain-specific concepts in the connections module.
  *
  * This object provides type-safe representations for various string-based
  * identifiers used throughout the connections module, improving type safety
  * while maintaining runtime efficiency through zero-cost abstractions.
  */
object ConnectionTypes:
  /** Represents a connection name in the system. */
  opaque type ConnectionName = String

  object ConnectionName:
    /** Creates a new ConnectionName from a string */
    def apply(name: String): ConnectionName =
      if name.isEmpty then
        throw new IllegalArgumentException("Connection name cannot be empty")
      name

    /** Empty connection name constant */
    val empty: ConnectionName = ""

    /** Safely converts a string to an Option[ConnectionName] */
    def from(name: String): Option[ConnectionName] =
      try Some(apply(name))
      catch case _: IllegalArgumentException => None

  extension (name: ConnectionName)
    /** Gets the underlying string value */
    @targetName("connectionNameValue")
    def value: String = name

    /** Checks if this connection name contains the given substring */
    def contains(substring: String): Boolean = name.contains(substring)

    /** Returns a new ConnectionName with the first letter capitalized */
    def capitalize: ConnectionName =
      ConnectionName(s"${name.charAt(0).toUpper}${name.substring(1)}")

    /** Equality comparison with a string */
    @targetName("connectionNameEquals")
    def ===(other: String): Boolean = name == other

  /** Represents a bus name in the system. */
  opaque type BusName = String

  object BusName:
    /** Creates a new BusName from a string */
    def apply(name: String): BusName = name

    /** Empty bus name constant */
    val empty: BusName = ""

    /** Default bus name constant */
    val default: BusName = "default"

  extension (name: BusName)
    /** Gets the underlying string value */
    @targetName("busNameValue")
    def value: String = name

    /** Checks if this bus name is empty */
    def isEmpty: Boolean = name.isEmpty

    /** Checks if this bus name is not empty */
    def nonEmpty: Boolean = name.length > 0

    /** Equality comparison with a string */
    @targetName("busNameEquals")
    def ===(other: String): Boolean = name == other

    /** String representation for testing */
    @targetName("busNameToString")
    def asString: String = name

  /** Represents a port name in the system. */
  opaque type PortName = String

  object PortName:
    /** Creates a new PortName from a string */
    def apply(name: String): PortName =
      if name.isEmpty then
        throw new IllegalArgumentException("Port name cannot be empty")
      name

  extension (name: PortName)
    /** Gets the underlying string value */
    @targetName("portNameValue")
    def value: String = name

    /** Checks if this port name starts with the given prefix */
    def startsWith(prefix: String): Boolean = name.startsWith(prefix)

    /** Returns a new PortName with the given prefix replaced */
    def replace(prefix: String, replacement: String): PortName =
      PortName(name.replace(prefix, replacement))

    /** Equality comparison with a string */
    @targetName("portNameEquals")
    def ===(other: String): Boolean = name == other

  /** Represents an instance name in the system. */
  opaque type InstanceName = String

  object InstanceName:
    /** Creates a new InstanceName from a string */
    def apply(name: String): InstanceName =
      if name.isEmpty then
        throw new IllegalArgumentException("Instance name cannot be empty")
      name

  extension (name: InstanceName)
    /** Gets the underlying string value */
    @targetName("instanceNameValue")
    def value: String = name

    /** Gets the last segment of the instance name (after the last dot) */
    def lastSegment: String = name.split('.').last

    /** Gets the first segment of the instance name (before the first dot) */
    def firstSegment: String = name.split('.').head

    /** Checks if this instance name is less than another instance name (for
      * sorting)
      */
    def <(other: InstanceName): Boolean = name.compareTo(other.toString) < 0

    /** Equality comparison with a string */
    @targetName("instanceNameEquals")
    def ===(other: String): Boolean = name == other

  // Implicit conversions between opaque types and String
  given Conversion[ConnectionName, String] = identity
  given Conversion[BusName, String] = identity
  given Conversion[PortName, String] = identity
  given Conversion[InstanceName, String] = identity

  // Implicit conversions from String to opaque types
  given Conversion[String, BusName] = BusName.apply
  given Conversion[String, ConnectionName] = ConnectionName.apply
  given Conversion[String, PortName] = PortName.apply
  given Conversion[String, InstanceName] = InstanceName.apply
