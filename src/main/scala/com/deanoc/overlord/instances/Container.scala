package com.deanoc.overlord.instances

import com.deanoc.overlord.interfaces.UnconnectedLike

trait Container {
  def children: Seq[InstanceTrait]
  def unconnected: Seq[UnconnectedLike]
  val physical: Boolean

  def flatChildren: Seq[InstanceTrait] =
    (children
      .filter(_.isInstanceOf[Container])
      .map(_.asInstanceOf[Container])
      .flatMap(_.flatChildren) ++ children).toSeq

  def chipChildren: Seq[ChipInstance] =
    children
      .filter(_.isInstanceOf[ChipInstance])
      .map(_.asInstanceOf[ChipInstance])
      .toSeq
}

case class MutableContainer() extends Container {
  override val physical: Boolean = true
  override def children: Seq[InstanceTrait] = mutableChildren
  override def unconnected: Seq[UnconnectedLike] = mutableUnconnected

  var mutableChildren: Seq[InstanceTrait] = Seq()
  var mutableUnconnected: Seq[UnconnectedLike] = Seq()
}
