package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.ChipDefinitionTrait

sealed trait RamFillType

case class ZeroFillType() extends RamFillType

case class PrimaryBootFillType() extends RamFillType

case class RamInstance(ident: String,
                       private val sizeInBytes: Option[BigInt],
                       fillType: RamFillType,
                       override val definition: ChipDefinitionTrait,
                      ) extends ChipInstance {
	override def copyMutate[A <: ChipInstance](nid: String): RamInstance =
		copy(ident = nid)

	def getSizeInBytes: BigInt = sizeInBytes match {
		case Some(value) => value
		case None        => Utils.lookupBigInt(attributes, "size_in_bytes", 1024)
	}

}

object RamInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
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