package com.deanoc.overlord

import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.instances.InstanceTrait

sealed trait PinConstraintType {
  val ports: Seq[HardwareBoundrary]
  val names: Seq[String]
  val directions: Seq[String]
  val pullups: Seq[Boolean]
}

case class PinConstraint(
    pins: Seq[String],
    ports: Seq[HardwareBoundrary],
    standard: String,
    names: Seq[String] = Seq(),
    directions: Seq[String] = Seq(),
    pullups: Seq[Boolean] = Seq()
) extends PinConstraintType

case class DiffPinConstraint(
    pins: Seq[(String, String)],
    ports: Seq[HardwareBoundrary],
    standard: String,
    names: Seq[String] = Seq(),
    directions: Seq[String] = Seq(),
    pullups: Seq[Boolean] = Seq()
) extends PinConstraintType
