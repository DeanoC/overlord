package overlord.Instances

import overlord.Definitions.DefinitionTrait
import toml.Value

case class NxMInstance(ident: String,
                       definition: DefinitionTrait,
                       attributes: Map[String, Value]
                      ) extends Instance {
	def copyMutate[A <: Instance](nid: String,
	                              nattribs: Map[String, Value]): NxMInstance =
		copy(ident = nid, attributes = nattribs)

	override def shared: Boolean = true
}
