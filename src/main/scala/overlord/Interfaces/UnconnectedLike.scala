package overlord.Interfaces

import overlord.ConnectionDirection
import overlord.QueryInterface
import overlord.Connections.{Connected, Constant}
import overlord.Instances.{ChipInstance, InstanceTrait}

trait UnconnectedLike extends QueryInterface {
  def direction: ConnectionDirection
  def preConnect(unexpanded: Seq[ChipInstance]): Unit
  def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit
  def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant]
  def connect(unexpanded: Seq[ChipInstance]): Seq[Connected]
}