package overlord

import toml._

import scala.collection.immutable.Map
import scala.collection.mutable.{ArrayBuffer, HashMap}

sealed trait ConstraintType

case class PinConstraint(pins: Seq[String]) extends ConstraintType

case class DiffPinConstraint(pins: Seq[(String, String)]) extends ConstraintType

case class ClockPinConstraint() extends ConstraintType

case class Constraint(
	                     name: String,
	                     constraintType: ConstraintType,
	                     attributes: Map[String, toml.Value]
                     )

object Constraint {
	def parse(
		         table: Map[String, toml.Value],
		         defaults: Map[String, toml.Value]
	         ): HashMap[String, Constraint] = {
		// are we a pin constraint?
		val hasPin = table.contains("pin")
		val hasPins = table.contains("pins")
		val hasDiffPin = table.contains("pin_p") && table.contains("pin_n")
		val hasDiffPins = table.contains("pin_ps") && table.contains("pin_ns")

		// is this a pin contraint of any type?
		if (hasPin || hasPins || hasDiffPin || hasDiffPins)
			parsePinConstraints(table, defaults)
		else {
			println(s"$table constraint isn't supported")
			HashMap[String, Constraint]()
		}
	}

	private def parsePinConstraints(
		                               table: Map[String, toml.Value],
		                               defaults: Map[String, toml.Value]
	                               ): HashMap[String, Constraint] = {
		val hasPin = table.contains("pin")
		val hasPins = table.contains("pins")
		val hasDiffPin = table.contains("pin_p") && table.contains("pin_n")
		val hasDiffPins = table.contains("pin_ps") && table.contains("pin_ns")
		val hasPrefix = table.contains("prefix")
		val hasDirections = table.contains("directions")
		val hasPullups = table.contains("pullups")

		var boardConstraints = HashMap[String, Constraint]()

		val prefix =
			if (hasPrefix) table("prefix").asInstanceOf[Value.Str].value + "." else ""

		val notSetDefaults = defaults.filterNot(d => table.contains(d._1)).toMap

		val attribs = (table ++ notSetDefaults).filter(_._1 match {
			case "prefix" | "name" | "names" => false
			case "pin" | "pin_p" | "pin_n" => false
			case "pins" | "pin_ps" | "pin_ns" => false
			case "direction" => if (hasDirections) false else true
			case "pullup" => if (hasPullups) false else true
			case _ => true
		})

		if (table.contains("name")) {
			val name = prefix + table("name").asInstanceOf[Value.Str].value

			if (hasPin) {
				val pin = table("pin").asInstanceOf[Value.Str].value
				boardConstraints += (name -> Constraint(
					name,
					PinConstraint(Seq(pin)),
					attribs
				))
			} else if (hasPins) {
				val pins = table("pins")
					.asInstanceOf[toml.Value.Arr]
					.values
					.map(_.asInstanceOf[Value.Str].value)
					.toSeq
				boardConstraints += (name -> Constraint(
					name,
					PinConstraint(pins),
					attribs
				))
			} else if (hasDiffPin) {
				val pins = (
					table("pin_p").asInstanceOf[Value.Str].value,
					table("pin_n").asInstanceOf[Value.Str].value
				)
				boardConstraints += (name -> Constraint(
					name,
					DiffPinConstraint(Seq(pins)),
					attribs
				))
			} else if (hasDiffPins) {
				val pin_ps = table("pin_ps")
					.asInstanceOf[Value.Arr]
					.values
					.map(_.asInstanceOf[Value.Str].value)
				val pin_ns = table("pin_ns")
					.asInstanceOf[toml.Value.Arr]
					.values
					.map(_.asInstanceOf[Value.Str].value)

				if (pin_ps.length != pin_ns.length) {
					println(
						s"${name} must have equal number of " +
							s"pin_ps(${pin_ps.length}) and pin_ns(${pin_ns.length})"
					)
					return boardConstraints
				}

				boardConstraints += (name -> Constraint(
					name,
					DiffPinConstraint(pin_ps.zip(pin_ns)),
					attribs
				))
			}

		} else if (table.contains("names")) {
			val names = table("names")
				.asInstanceOf[Value.Arr]
				.values
				.map(prefix + _.asInstanceOf[Value.Str].value)

			if (!(hasPins || hasDiffPins)) {
				println(
					s"${names} contraint must have " +
						s"'pins' or 'pin_ps' and 'pin_ns' field"
				)
				return boardConstraints
			}

			if (hasPins) {
				val pins = table("pins")
					.asInstanceOf[Value.Arr]
					.values
					.map(_.asInstanceOf[Value.Str].value)
				if (names.length != pins.length) {
					println(
						s"${names} must have equal number of " +
							s"names(${names.length}) and pins(${pins.length})"
					)
					return boardConstraints
				}

				val perNameAttribs = ArrayBuffer[(String, Value)]()

				if (table.contains("directions"))
					perNameAttribs ++= table("directions")
						.asInstanceOf[Value.Arr]
						.values
						.map(v => ("direction", v.asInstanceOf[Value.Str]))

				if (table.contains("pullups"))
					perNameAttribs ++= table("pullups")
						.asInstanceOf[Value.Arr]
						.values
						.map(v => ("pull", v.asInstanceOf[Value.Bool]))

				for ((nm, (p, (anm, v))) <- names.zip(pins.zip(perNameAttribs)))
					boardConstraints += (s"$nm" -> Constraint(
						s"$nm",
						PinConstraint(Seq(p)),
						(attribs ++ Map(anm -> v))
					))

			} else if (hasDiffPins) {
				val pin_ps = table("pin_ps")
					.asInstanceOf[Value.Arr]
					.values
					.map(_.asInstanceOf[Value.Str].value)
				val pin_ns = table("pin_ns")
					.asInstanceOf[Value.Arr]
					.values
					.map(_.asInstanceOf[Value.Str].value)
				if (names.length != pin_ps.length || pin_ps.length != pin_ns.length) {
					println(
						s"""|${names} must have equal number of
						    |names(${names.length}), pin_ps(${pin_ps.length}) and
						    |pin_ns(${pin_ns.length})""".stripMargin
					);
					return boardConstraints
				}

				for ((nm, d) <- names.zip(pin_ps.zip(pin_ns)))
					boardConstraints += (nm -> Constraint(
						nm,
						DiffPinConstraint(Seq(d)),
						attribs
					))
			}
		}
		boardConstraints
	}
}
