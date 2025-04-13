package com.deanoc.overlord

enum ConnectionDirection {
  case FirstToSecond, SecondToFirst, BiDirectional

  override def toString: String = this match {
    case FirstToSecond => "first to second"
    case SecondToFirst => "second to first"
    case BiDirectional => "bi direction"
  }

  def flip: ConnectionDirection = this match {
    case FirstToSecond => SecondToFirst
    case SecondToFirst => FirstToSecond
    case BiDirectional => this // BiDirectional remains unchanged
  }
}
