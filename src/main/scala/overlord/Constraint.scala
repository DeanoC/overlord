package overlord

import overlord.Hardware.Port
import overlord.Connections._
import overlord.Instances.InstanceTrait

sealed trait PinConstraintType{
	val ports: Seq[Port]
	val names: Seq[String]
	val directions: Seq[String]
	val pullups: Seq[Boolean]
}

case class PinConstraint( pins: Seq[String],
                          ports: Seq[Port],
                          standard: String,
                          names: Seq[String]= Seq(),
                          directions: Seq[String]= Seq(),
                          pullups: Seq[Boolean]= Seq())
	extends PinConstraintType

case class DiffPinConstraint( pins: Seq[(String, String)],
                              ports: Seq[Port],
                              standard: String,
                              names: Seq[String] = Seq(),
                              directions: Seq[String]= Seq(),
                              pullups: Seq[Boolean]= Seq())
	extends PinConstraintType
