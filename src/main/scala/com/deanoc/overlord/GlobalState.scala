package com.deanoc.overlord

object GlobalState {
  // default to not allowing writes to the project (logs/info are always allowed)
  private var ReadOnly: Boolean = true
  
  def allowWrites(): Unit = {
    ReadOnly = false
  }

  def isReadOnly: Boolean = ReadOnly
}
