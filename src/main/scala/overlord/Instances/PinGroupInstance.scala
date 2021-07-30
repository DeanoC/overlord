package overlord.Instances

import overlord.Chip.{BitsDesc, Port, WireDirection}
import overlord.{ChipDefinitionTrait, DiffPinConstraint, PinConstraint, PinConstraintType}
import ikuy_utils._
import toml.Value

import scala.collection.immutable.Map
import scala.collection.mutable

case class PinGroupInstance(ident: String,
                            constraint: PinConstraintType,
                            override val definition: ChipDefinitionTrait,
                           ) extends ChipInstance {

	override def copyMutate[A <: ChipInstance](nid: String): PinGroupInstance =
		copy(ident = nid)

	override lazy val ports: mutable.HashMap[String, Port] =
		mutable.HashMap.from(definition.ports ++ constraint.ports.map(p => p.name -> p))
}

object PinGroupInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attributes: Map[String, Variant]): Option[PinGroupInstance] = {

		val hasPin      = attributes.contains("pin")
		val hasPins     = attributes.contains("pins")
		val hasDiffPin  = attributes.contains("pin_p") &&
		                  attributes.contains("pin_n")
		val hasDiffPins = attributes.contains("pin_ps") &&
		                  attributes.contains("pin_ns")
		val hasPrefix   = attributes.contains("prefix")

		val hasDirection = attributes.contains("direction")
		val hasPullup    = attributes.contains("pullup")
		val hasName      = attributes.contains("name")

		val hasDirections = attributes.contains("directions")
		val hasPullups    = attributes.contains("pullups")
		val hasNames      = attributes.contains("names")

		if (!(hasPin || hasPins || hasDiffPin || hasDiffPins)) {
			println(s"$ident is a pin constraint without pins?")
			return None
		}

		val attribs = attributes.filter(
			_._1 match {
				case "type"                       => false
				case "prefix" | "name" | "names"  => false
				case "pin" | "pin_p" | "pin_n"    => false
				case "pins" | "pin_ps" | "pin_ns" => false
				case "direction" | "directions"   => false
				case "pullup" | "pullups"         => false
				case _                            => true
			})

		val name: String = {
			val pf =
				if (hasPrefix) Utils.toString(attributes("prefix")) + "."
				else ""

			val nm =
				if (hasName) Utils.toString(attributes("name"))
				else if (hasNames) ""
				else ident

			pf + nm
		}

		val pins = if (hasPin) Array(Utils.toString(attributes("pin")))
		else if (hasPins) Utils.toArray(attributes("pins")).map(Utils.toString)
		else Array("")

		val diffPins = if (hasDiffPin)
			Array((Utils.toString(attributes("pin_p")),
				      Utils.toString(attributes("pin_n"))))
		else if (hasDiffPins) {
			val pin_ps = Utils.toArray(attributes("pin_ps")).map(Utils.toString)
			val pin_ns = Utils.toArray(attributes("pin_ns")).map(Utils.toString)

			pin_ps.zip(pin_ns)
		} else Array(("",""))

		val pinCount = Math.max(pins.length, diffPins.length)

		// validate arrays are all the same size
		if (pinCount > 1) {
			val pinNameCount = if (hasNames)
				Utils.toArray(attributes("names")).length else pinCount
			val dirCount     = if (hasDirections)
				Utils.toArray(attributes("directions")).length else pinCount
			val pullupCount  = if (hasPullups)
				Utils.toArray(attributes("pullups")).length else pinCount

			if (pinCount != pinNameCount) {
				println(s"$name must have equal number of " +
				        s"pin names($pinNameCount) and pins($pinCount)")
				return None
			}
			if (pinCount != dirCount) {
				println(s"$name must have equal number of " +
				        s"pin directions($dirCount) and pins($pinCount)")
				return None
			}
			if (pinCount != pullupCount) {
				println(s"$name must have equal number of " +
				        s"pin pullups($pullupCount) and pins($pinCount)")
				return None
			}
		}
		val standard = Utils.lookupString(attributes, "standard", "LVCMOS33")

		val pinNames = if (hasNames)
			Utils.toArray(attributes("names"))
				.map(Utils.toString)
				.map(name + _)
		else Array(name)

		val constraintPinNames = if (hasNames)
			Utils.toArray(attributes("names"))
				.map(Utils.toString)
				.map(name + _)
		else if (pinCount > 1)
			(for (i <- 0 until pinCount) yield name + s"[$i]").toArray
		else Array(name)

		val directions = if (hasDirection) {
			val dir = Utils.toString(attributes("direction")).toLowerCase
			Array.fill(pinCount)(dir)
		} else if (hasDirections)
			Utils.toArray(attributes("directions"))
				.map(Utils.toString)
				.map(_.toLowerCase)
		else Array.fill(pinCount)("inout")

		val pullups = if (hasPullup) {
			val pullup = Utils.toBoolean(attributes("pullup"))
			Array.fill(pinCount)(pullup)
		} else if (hasPullups)
			Utils.toArray(attributes("pullups")).map(Utils.toBoolean)
		else Array.fill(pinCount)(false)

		val ports = if (pinNames.length > 1)
			for ((nm, i) <- pinNames.zipWithIndex) yield
				Port(nm.split('.').last, BitsDesc(1), WireDirection(directions(i)))
		else Array(
			Port(pinNames.head, BitsDesc(pinCount), WireDirection(directions.head)))

		val constraint = if (hasPin || hasPins)
			PinConstraint(pins.toSeq,
			              ports.toSeq,
			              standard,
			              constraintPinNames.toSeq,
			              directions.toSeq,
			              pullups.toSeq)
		else if (hasDiffPin || hasDiffPins)
			DiffPinConstraint(diffPins.toSeq,
			                  ports.toSeq,
			                  standard,
			                  constraintPinNames.toSeq,
			                  directions.toSeq,
			                  pullups.toSeq)
		else return None

		Some(PinGroupInstance(name,
		                      constraint,
		                      definition))
	}
}

