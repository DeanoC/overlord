package overlord

import overlord.Definitions.{Definition, PortDefinitionType}
import overlord.Instances.ConstraintInstance
import toml._

import scala.collection.immutable.Map
import scala.collection.mutable

sealed trait ConstraintType

case class PinConstraint(pins: Seq[String]) extends ConstraintType

case class DiffPinConstraint(pins: Seq[(String, String)]) extends ConstraintType

case class ClockPinConstraint() extends ConstraintType


object Constraint {
	def parse(table: Map[String, toml.Value],
	          defaults: Map[String, toml.Value]
	         ): mutable.HashMap[String, ConstraintInstance] = {
		// are we a pin constraint?
		val hasPin      = table.contains("pin")
		val hasPins     = table.contains("pins")
		val hasDiffPin  = table.contains("pin_p") && table.contains("pin_n")
		val hasDiffPins = table.contains("pin_ps") && table.contains("pin_ns")

		// is this a pin contraint of any type?
		if (hasPin || hasPins || hasDiffPin || hasDiffPins)
			parsePinConstraints(table, defaults)
		else {
			println(s"$table constraint isn't supported")
			mutable.HashMap[String, ConstraintInstance]()
		}
	}

	private def parsePinConstraints(table: Map[String, Value],
	                                defaults: Map[String, Value]
	                               ): mutable.HashMap[String,
		ConstraintInstance] = {

		val hasPin        = table.contains("pin")
		val hasPins       = table.contains("pins")
		val hasDiffPin    = table.contains("pin_p") && table.contains("pin_n")
		val hasDiffPins   = table.contains("pin_ps") && table.contains("pin_ns")
		val hasPrefix     = table.contains("prefix")
		val hasDirections = table.contains("directions")
		val hasPullups    = table.contains("pullups")

		var boardConstraints = mutable.HashMap[String, ConstraintInstance]()

		val prefix =
			if (hasPrefix) Utils.toString(table("prefix")) + "."
			else ""

		val notSetDefaults = defaults.filterNot(d => table.contains(d._1)).toMap

		val attribs = (table ++ notSetDefaults).filter(
			_._1 match {
				case "prefix" | "name" | "names"  => false
				case "pin" | "pin_p" | "pin_n"    => false
				case "pins" | "pin_ps" | "pin_ns" => false
				case "direction"                  => if (hasDirections) false else true
				case "pullup"                     => if (hasPullups) false else true
				case _                            => true
			})

		if (table.contains("name")) {
			val name = prefix + Utils.toString(table("name"))

			val defi  = Definition(PortDefinitionType(name.split('.'), Seq()), attribs)

			val cType = if (hasPin)
				PinConstraint(Seq(Utils.toString(table("pin"))))
			else if (hasPins)
				PinConstraint(Utils.toArray(table("pins")).map(Utils.toString))
			else if (hasDiffPin)
				DiffPinConstraint(Seq((Utils.toString(table("pin_p")),
					                      Utils.toString(table("pin_n")))))
			else if (hasDiffPins) {
				val pin_ps = Utils.toArray(table("pin_ps")).map(Utils.toString)
				val pin_ns = Utils.toArray(table("pin_ns")).map(Utils.toString)
				if (pin_ps.length != pin_ns.length) {
					println(s"${name} must have equal number of " +
					        s"pin_ps(${pin_ps.length}) and pin_ns(${pin_ns.length})")
					return boardConstraints
				}
				DiffPinConstraint(pin_ps.zip(pin_ns))
			} else {
				println(s"$name is a pin constraint without pins?")
				return boardConstraints
			}

			boardConstraints +=
			(name -> ConstraintInstance(name, defi, cType, attribs))

		} else if (table.contains("names")) {

			val names = Utils.toArray(table("names")).map(prefix + Utils.toString(_))

			if (!(hasPins || hasDiffPins)) {
				println(s"${names} constraint must have " +
				        s"'pins' or 'pin_ps' and 'pin_ns' field")
				return boardConstraints
			}

			if (hasPins) {
				val pins = Utils.toArray(table("pins")).map(Utils.toString)

				if (names.length != pins.length) {
					println(s"${names} must have equal number of " +
					        s"names(${names.length}) and pins(${pins.length})")
					return boardConstraints
				}

				val perNameAttribs = mutable.ArrayBuffer[(String, Value)]()

				if (table.contains("directions"))
					perNameAttribs ++= Utils.toArray(table("directions"))
						.map(v => ("direction", v.asInstanceOf[Value.Str]))

				if (table.contains("pullups"))
					perNameAttribs ++= Utils.toArray(table("pullups"))
						.map(v => ("pull", v.asInstanceOf[Value.Bool]))

				for ((nm, (p, (anm, v))) <- names.zip(pins.zip(perNameAttribs))) {
					boardConstraints +=
					(nm -> ConstraintInstance(nm,
					                          Definition(
						                          PortDefinitionType(nm.split('.'), Seq()),
						                          attribs),
					                          PinConstraint(Seq(p)),
					                          (attribs ++ Map(anm -> v))))
				}
			} else if (hasDiffPins) {
				val pin_ps = Utils.toArray(table("pin_ps")).map(Utils.toString)
				val pin_ns = Utils.toArray(table("pin_ns")).map(Utils.toString)

				if (names.length != pin_ps.length || pin_ps.length != pin_ns.length) {
					println(
						s"""|${names} must have equal number of
						    |names(${names.length}), pin_ps(${pin_ps.length}) and
						    |pin_ns(${pin_ns.length})""".stripMargin
						);
					return boardConstraints
				}

				for ((nm, d) <- names.zip(pin_ps.zip(pin_ns))) {
					boardConstraints +=
					(nm -> ConstraintInstance(nm,
					                          Definition(
						                          PortDefinitionType(nm.split('.'), Seq()),
						                          attribs),
					                          DiffPinConstraint(Seq(d)),
					                          attribs))

				}
			}
		}
		boardConstraints
	}
}
