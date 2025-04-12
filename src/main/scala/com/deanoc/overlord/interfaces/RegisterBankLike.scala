package com.deanoc.overlord.Interfaces

import com.deanoc.overlord.QueryInterface
import com.deanoc.overlord.Hardware.RegisterBank

trait RegisterBankLike extends QueryInterface {
  def maxInstances: Int
  def banks: Seq[RegisterBank]
}
