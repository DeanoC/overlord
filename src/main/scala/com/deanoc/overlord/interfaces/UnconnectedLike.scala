package com.deanoc.overlord.Interfaces

import com.deanoc.overlord.ConnectionDirection
import com.deanoc.overlord.QueryInterface
import com.deanoc.overlord.Connections.{Connected, Constant}
import com.deanoc.overlord.Instances.{ChipInstance, InstanceTrait}

trait UnconnectedLike extends QueryInterface {
  def direction: ConnectionDirection
  def preConnect(unexpanded: Seq[ChipInstance]): Unit
  def finaliseBuses(unexpanded: Seq[ChipInstance]): Unit
  def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant]
  def connect(unexpanded: Seq[ChipInstance]): Seq[Connected]
}
