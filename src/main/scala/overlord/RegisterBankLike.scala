package overlord

import overlord.Hardware.RegisterBank

trait RegisterBankLike extends QueryInterface {
  def maxInstances: Int
  def banks: Seq[RegisterBank]
}