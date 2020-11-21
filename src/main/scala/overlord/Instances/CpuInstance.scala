package overlord.Instances

import overlord.Definitions.DefinitionTrait
import toml.Value

case class CpuInstance(ident: String,
                       definition: DefinitionTrait,
                       attributes: Map[String, Value],
                      ) extends Instance {
	def copyMutate[A <: Instance](nid: String,
	                        nattribs: Map[String, Value]): CpuInstance =
		copy(ident = nid, attributes = nattribs)

}
