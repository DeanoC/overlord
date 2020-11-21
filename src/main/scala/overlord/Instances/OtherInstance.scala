package overlord.Instances

import overlord.Definitions.DefinitionTrait
import toml.Value

case class OtherInstance(ident: String,
                         definition: DefinitionTrait,
                         attributes: Map[String, toml.Value]
                        ) extends Instance {
	def copyMutate[A <: Instance](nid: String,
	                              nattribs: Map[String, Value]): OtherInstance =
		copy(ident = nid, attributes = nattribs)

}
