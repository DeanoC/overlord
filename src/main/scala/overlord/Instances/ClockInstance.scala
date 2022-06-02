package overlord.Instances

import ikuy_utils._
import overlord.ChipDefinitionTrait

case class ClockInstance(name: String,
                         override val definition: ChipDefinitionTrait)
	extends ChipInstance {

	lazy val pin     : String =
		Utils.lookupString(attributes, "pin", or = "INVALID")
	lazy val standard: String =
		Utils.lookupString(attributes, "standard", or = "LVCMOS33")
	lazy val period  : Double =
		Utils.lookupDouble(attributes, "period", 10.0)
	lazy val waveform: String =
		Utils.lookupString(attributes, "waveform", "{0 5}")

}

object ClockInstance {
	def apply(name: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]): Option[ClockInstance] = {

		if(!definition.attributes.contains("pin") && attribs.contains("pin")) {
			println(f"$name is a clock without a pin attribute")
			None
		} else {
			val clock  = ClockInstance(name, definition)
			clock.mergeAttribute(attribs, "pin")
			clock.mergeAttribute(attribs, "standard")
			clock.mergeAttribute(attribs, "period")
			clock.mergeAttribute(attribs, "waveform")

			Some(clock)
		}
	}
}
