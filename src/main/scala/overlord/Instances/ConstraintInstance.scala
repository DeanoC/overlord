package overlord.Instances

import overlord.{Constraint, ConstraintType}
import overlord.Definitions.DefinitionTrait
import toml.Value

import scala.collection.immutable.Map

case class ConstraintInstance(ident: String,
                              definition: DefinitionTrait,
                              constraintType: ConstraintType,
                              attributes: Map[String, Value] =
                              Map[String, Value](),
                             ) extends Instance {
	def copyMutate[A <: Instance](nid: String,
	                        nattribs: Map[String, Value])
	: ConstraintInstance =
		copy(ident = nid, attributes = nattribs)
}
