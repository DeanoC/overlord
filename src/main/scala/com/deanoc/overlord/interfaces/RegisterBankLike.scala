package com.deanoc.overlord.interfaces

import com.deanoc.overlord.QueryInterface
import com.deanoc.overlord.hardware.RegisterBank

trait RegisterBankLike extends QueryInterface {
  def maxInstances: Int
  def banks: Seq[RegisterBank]
}
