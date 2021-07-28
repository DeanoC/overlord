package overlord.Instances

import overlord.Definitions.DefinitionTrait
import overlord.Gateware.{BitsDesc, Port}
import ikuy_utils._
import toml.Value

case class ClockInstance(ident: String,
                         private val defi: DefinitionTrait)
	extends Instance {

	lazy val pin: String =
		Utils.lookupString(attributes, "pin", or = "INVALID")
	lazy val standard: String =
		Utils.lookupString(attributes, "standard", or = "LVCMOS33")
	lazy val period: Double   =
		Utils.lookupDouble(attributes, "period", 10.0)
	lazy val waveform: String =
		Utils.lookupString(attributes, "waveform", "{0 5}")

	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): ClockInstance =
		copy(ident = nid)

}

object ClockInstance {
	def apply(name: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]): Option[ClockInstance] = {

		if(!definition.attributes.contains("pin") && attribs.contains("pin")) {
			println(f"$name is a clock without a pin attribute")
			None
		} else {
			val clock  = ClockInstance(name, definition)
			clock.mergeParameter(attribs, "pin")
			clock.mergeParameter(attribs, "standard")
			clock.mergeParameter(attribs, "period")
			clock.mergeParameter(attribs, "waveform")

			Some(clock)
		}
	}
}
