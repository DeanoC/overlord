package overlord.Instances

import overlord.Definitions.DefinitionTrait
import toml.Value

case class BridgeInstance(ident: String,
                          definition: DefinitionTrait,
                          attributes: Map[String, Value],
                         ) extends Instance {
	def copyMutate[A <: Instance](nid: String,
	                              nattribs: Map[String, Value]): BridgeInstance =
		copy(ident = nid, attributes = nattribs)

}
