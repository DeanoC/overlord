package overlord.Instances

import overlord.Definitions.DefinitionTrait
import toml.Value

case class SocInstance(ident: String,
                       definition: DefinitionTrait,
                       attributes: Map[String, Value]
                      ) extends Instance {
	def copyMutate[A <: Instance](nid: String,
	                              nattribs: Map[String, Value]): SocInstance =
		copy(ident = nid, attributes = nattribs)

}
