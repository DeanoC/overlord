package com.deanoc.overlord.Connections

// Replace sealed trait + case objects with enum
enum ConnectionPriority:
  case High, Medium, Low
  
  // Add helper methods within the enum
  def toInt: Int = this match
    case High => 3
    case Medium => 2
    case Low => 1
  
  // Compare priorities
  def >(other: ConnectionPriority): Boolean = this.toInt > other.toInt
  def <(other: ConnectionPriority): Boolean = this.toInt < other.toInt
  def >=(other: ConnectionPriority): Boolean = this.toInt >= other.toInt
  def <=(other: ConnectionPriority): Boolean = this.toInt <= other.toInt
  
// Companion object for additional functionality
object ConnectionPriority:
  // Convert from string representation
  def fromString(str: String): Option[ConnectionPriority] = str.toLowerCase match
    case "high" => Some(High)
    case "medium" => Some(Medium)
    case "low" => Some(Low)
    case _ => None
    
  // Default priority
  def default: ConnectionPriority = Medium
