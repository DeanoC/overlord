package com.deanoc.overlord

sealed trait ConnectionDirection {
  override def toString: String =
    this match {
      case FirstToSecondConnection() => "first to second"
      case SecondToFirstConnection() => "second to first"
      case BiDirectionConnection()   => "bi direction"
    }

  def flip: ConnectionDirection = this match {
    case FirstToSecondConnection() => SecondToFirstConnection()
    case SecondToFirstConnection() => FirstToSecondConnection()
    case BiDirectionConnection()   => this
  }
}

case class FirstToSecondConnection() extends ConnectionDirection

case class SecondToFirstConnection() extends ConnectionDirection

case class BiDirectionConnection() extends ConnectionDirection
