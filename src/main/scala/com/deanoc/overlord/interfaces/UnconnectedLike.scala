package com.deanoc.overlord.interfaces

import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.QueryInterface
import com.deanoc.overlord.connections.{Connected, Constant}
import com.deanoc.overlord.instances.{HardwareInstance, InstanceTrait}

trait UnconnectedLike extends QueryInterface {
  def direction: ConnectionDirection
  def preConnect(unexpanded: Seq[HardwareInstance]): Unit
  def finaliseBuses(unexpanded: Seq[HardwareInstance]): Unit
  def collectConstants(unexpanded: Seq[InstanceTrait]): Seq[Constant]
  def connect(unexpanded: Seq[HardwareInstance]): Seq[Connected]
}
