package overlord.Instances

import overlord.Definitions.DefinitionTrait
import overlord.Gateware.{BitsDesc, Port}
import ikuy_utils._
import toml.Value

case class ClockInstance(ident: String,
                         pin: String,
                         standard: String,
                         period: Double,
                         waveform: String,
                         private val defi: DefinitionTrait)
	extends Instance {

	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): ClockInstance =
		copy(ident = nid)

	override def getPort(lastName: String): Option[Port] = {
		// TODO replace this hack
		Some(Port(lastName, BitsDesc(1)))
		/*if(lastName == "clk") Some(Port(lastName, BitsDesc(1)))
		else None*/
	}

	override def ports: Map[String, Port] =
		Map[String, Port]("clk" -> Port("clk", BitsDesc(1)))

}

object ClockInstance {
	def apply(name: String,
	          definition: DefinitionTrait,
	          attributes: Map[String, Variant]): Option[ClockInstance] = {
		if (!attributes.contains("pin")) {
			println(s"$name clock doesn't contain a pin")
			None
		} else {
			val standard = Utils.lookupString(attributes, "standard", "LVCMOS33")
			val period   = Utils.lookupDouble(attributes, "period", 10.0)
			val waveform = Utils.lookupString(attributes, "waveform", "{0 5}")

			Some(ClockInstance(name,
			                   Utils.toString(attributes("pin")),
			                   standard,
			                   period,
			                   waveform,
			                   definition))
		}
	}
}
