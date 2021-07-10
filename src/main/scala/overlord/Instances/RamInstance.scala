package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.Definitions.DefinitionTrait

sealed trait RamFillType

case class ZeroFillType() extends RamFillType

case class PrimaryBootFillType() extends RamFillType

case class RamInstance(ident: String,
                       sizeInBytes: Option[BigInt],
                       fillType: RamFillType,
                       private val defi: DefinitionTrait
                      ) extends Instance {
	override def definition: DefinitionTrait = defi

	override def copyMutate[A <: Instance](nid: String): RamInstance =
		copy(ident = nid)

	def getSizeInBytes: BigInt = sizeInBytes match {
		case Some(value) => value
		case None        => Utils.lookupBigInt(attributes, "size_in_bytes", 1024)
	}

}

object RamInstance {
	def apply(ident: String,
	          definition: DefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[RamInstance] = {

		val sizeInBytes    = if (attribs.contains("size_in_bytes"))
			Some(Utils.toBigInt(attribs("size_in_bytes")))
		else None
		val fillTypeString = Utils.lookupString(attribs,
		                                        "fill",
		                                        "zero")
		val fillType       = fillTypeString.toLowerCase match {
			case "primary_boot" => PrimaryBootFillType()
			case "zero" | _     => ZeroFillType()
		}

		Some(RamInstance(ident, sizeInBytes, fillType, definition))
	}
}