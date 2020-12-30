package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.Definitions.DefinitionTrait
import toml.Value

case class RamInstance(ident: String,
                       sizeInBytes: Option[BigInt],
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition:DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): RamInstance =
		copy(ident = nid)

}
object RamInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[RamInstance] = {

		val sizeInBytes =  if(attribs.contains("size_in_bytes"))
			Some(Utils.toBigInt(attribs("size_in_bytes")))
		else None

		Some(RamInstance(ident, sizeInBytes, definition))
	}
}