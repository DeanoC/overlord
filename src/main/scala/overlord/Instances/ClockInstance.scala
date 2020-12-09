package overlord.Instances

import overlord.Definitions.DefinitionTrait
import overlord.Gateware.{BitsDesc, Port}
import overlord.Utils
import toml.Value

case class ClockInstance(ident: String,
                         pin: String,
                         definition: DefinitionTrait,
                         attributes: Map[String, Value])
	extends Instance {

	override def copyMutate[A <: Instance](nid: String,
	                                       nattribs: Map[String, Value])
	: ClockInstance = copy(ident = nid, attributes = nattribs)

	override def getPort(lastName: String): Option[Port] = {
		// TODO replace this hack
		Some(Port(lastName, BitsDesc(1)))
		/*if(lastName == "clk") Some(Port(lastName, BitsDesc(1)))
		else None*/
	}

	override def getPorts: Map[String, Port] =
		Map[String, Port]("clk" -> Port("clk", BitsDesc(1)))

}

object ClockInstance {
	def apply(name: String,
	          definition: DefinitionTrait,
	          attributes: Map[String, Value]): Option[ClockInstance] = {
		if (!attributes.contains("pin")) {
			println(s"$name clock doesn't contain a pin")
			None
		} else {
			Some(ClockInstance(name,
			                   Utils.toString(attributes("pin")),
			                   definition = definition,
			                   attributes = attributes))
		}
	}
}
