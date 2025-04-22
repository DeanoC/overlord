package com.deanoc.overlord.config

// Enum representing the direction of a wire
enum WireDirection {
  case Input, Output, InOut
  
  override def toString: String = this match {
    case Input  => "input"
    case Output => "output"
    case InOut  => "inout"
  }
  
  def flip: WireDirection = this match {
    case Input  => Output
    case Output => Input
    case InOut  => InOut
  }
}

object WireDirection {
  def apply(s: String): WireDirection = {
    s.toLowerCase match {
      case "in" | "input"   => WireDirection.Input
      case "out" | "output" => WireDirection.Output
      case "inout" | "bi"   => WireDirection.InOut
      case _ =>
        println(s"$s is an invalid port direction")
        WireDirection.InOut
    }
  }
}
