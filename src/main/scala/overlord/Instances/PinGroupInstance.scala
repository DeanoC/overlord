package overlord.Instances

import overlord.Definitions.DefinitionTrait
import overlord.Gateware.{BitsDesc, Port, WireDirection}
import overlord.{DiffPinConstraint, PinConstraint, PinConstraintType}
import ikuy_utils._
import toml.Value

import scala.collection.immutable.Map

case class PinGroupInstance(ident: String,
                            constraint: PinConstraintType,
                            definition: DefinitionTrait,
                            attributes: Map[String, Value]
                           ) extends Instance {
	override def copyMutate[A <: Instance](nid: String,
	                                       nattribs: Map[String, Value])
	: PinGroupInstance =
		copy(ident = nid, attributes = nattribs)

	private lazy val allPorts: Map[String, Port] =
		super.getPorts ++ constraint.ports.map(p => (p.name -> p))

	override def getPorts: Map[String,Port] = allPorts
}

object PinGroupInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attributes: Map[String, Value]): Option[PinGroupInstance] = {

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

		val prefix =
			if (hasPrefix) Utils.toString(attributes("prefix")) + "."
			else ""

		val attribs = attributes.filter(
			_._1 match {
				case "type"                       => false
				case "prefix" | "name" | "names"  => false
				case "pin" | "pin_p" | "pin_n"    => false
				case "pins" | "pin_ps" | "pin_ns" => false
				case "direction" | "directions"   =>
					if (hasDirections || hasDirection) false else true
				case "pullup" | "pullups"         =>
					if (hasPullups || hasPullup) false else true
				case _                            => true
			})

		val name = prefix +
		           (if (hasName) Utils.toString(attributes("name")) else ident)

		val pins = if (hasPin) Seq(Utils.toString(attributes("pin")))
		else if (hasPins) Utils.toArray(attributes("pins")).map(Utils.toString)
		else Seq()

		val diffPins = if (hasDiffPin)
			Seq((Utils.toString(attributes("pin_p")),
				    Utils.toString(attributes("pin_n"))))
		else if (hasDiffPins) {
			val pin_ps = Utils.toArray(attributes("pin_ps")).map(Utils.toString)
			val pin_ns = Utils.toArray(attributes("pin_ns")).map(Utils.toString)

			pin_ps.zip(pin_ns)
		} else Seq()

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
				println(s"${name} must have equal number of " +
				        s"pin names(${pinNameCount}) and pins(${pinCount})")
				return None
			}
			if (pinCount != dirCount) {
				println(s"${name} must have equal number of " +
				        s"pin directions(${dirCount}) and pins(${pinCount})")
				return None
			}
			if (pinCount != pullupCount) {
				println(s"${name} must have equal number of " +
				        s"pin pullups(${pullupCount}) and pins(${pinCount})")
				return None
			}
		}

		val pinNames = if (hasNames)
			Utils.toArray(attributes("names"))
				.map(Utils.toString)
				.map(prefix + _)
		else Seq(name)

		val constraintPinNames = if (hasNames)
			Utils.toArray(attributes("names"))
				.map(Utils.toString)
				.map(prefix + _)
		else if (pinCount > 1) for (i <- 0 until pinCount) yield name + s"[$i]"
		else Seq(name)

		val directions = if (hasDirection) {
			val dir = Utils.toString(attributes("direction"))
				.toLowerCase
			for (i <- 0 until pinCount) yield dir
		} else if (hasDirections)
			Utils.toArray(attributes("directions"))
				.map(Utils.toString)
				.map(_.toLowerCase)
		else for (i <- 0 until pinCount) yield "inout"

		val pullups = if (hasPullup) {
			val pullup = Utils.toBoolean(attributes("pullup"))
			for (i <- 0 until pinCount) yield pullup
		} else if (hasPullups)
			Utils.toArray(attributes("pullups")).map(Utils.toBoolean)
		else for (i <- 0 until pinCount) yield false

		val ports = if (pinNames.length > 1)
			for ((nm, i) <- pinNames.zipWithIndex) yield
				Port(nm.split('.').last, BitsDesc(1), WireDirection(directions(i)))
		else Seq(
			Port(pinNames.head, BitsDesc(pinCount), WireDirection(directions.head)))

		val constraint = if (hasPin || hasPins)
			PinConstraint(pins, ports, constraintPinNames, directions, pullups)
		else if (hasDiffPin || hasDiffPins)
			DiffPinConstraint(diffPins,
			                  ports,
			                  constraintPinNames,
			                  directions,
			                  pullups)
		else return None

		Some(PinGroupInstance(name,
		                      constraint,
		                      definition,
		                      attribs))
	}
}

