package com.deanoc.overlord.connections

/**
 * Represents the priority level of a connection, used to resolve conflicts
 * when multiple connection rules apply to the same components.
 */
enum ConnectionPriority:
  /** Priority for explicitly defined connections, which take precedence over others. */
  case Explicit
  
  /** Priority for connections made through grouped components, such as pin groups. */
  case Group
  
  /** Priority for wildcard connections, typically used as a fallback. */
  case WildCard
  
  /** Priority for placeholder or simulated connections, often used in testing. */
  case Fake
  
  /** Get numeric representation of priority for comparison */
  def toInt: Int = this match
    case Explicit => 4
    case Group => 3
    case WildCard => 2
    case Fake => 1
  
  /** Compare priorities */
  def >(other: ConnectionPriority): Boolean = this.toInt > other.toInt
  def <(other: ConnectionPriority): Boolean = this.toInt < other.toInt
  def >=(other: ConnectionPriority): Boolean = this.toInt >= other.toInt
  def <=(other: ConnectionPriority): Boolean = this.toInt <= other.toInt

/** Companion object for ConnectionPriority */
object ConnectionPriority:
  /** Default priority for connections */
  def default: ConnectionPriority = WildCard
  
  /**
   * Parse a string into a ConnectionPriority
   *
   * @param str The string representation of the priority
   * @return An Option containing the ConnectionPriority or None if invalid
   */
  def fromString(str: String): Option[ConnectionPriority] = str.toLowerCase match
    case "explicit" => Some(Explicit)
    case "group" => Some(Group)
    case "wildcard" => Some(WildCard)
    case "fake" => Some(Fake)
    case _ => None

