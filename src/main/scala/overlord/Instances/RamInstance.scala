package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.ChipDefinitionTrait
import overlord.Interfaces.RamLike

import scala.reflect.ClassTag

case class RamInstance(name: String,
                       override val definition: ChipDefinitionTrait,
                      ) extends ChipInstance with RamLike {
	private lazy val ranges: Seq[(BigInt, BigInt)] = {
		if (!attributes.contains("ranges")) Seq()
		else Utils.toArray(attributes("ranges")).map(
			b => (Utils.lookupBigInt(Utils.toTable(b), "address", 0), Utils.lookupBigInt(Utils.toTable(b), "size", 0)))
	}

	override def isVisibleToSoftware: Boolean = true

	override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
		val RamLike_ = classOf[RamLike]
		tag.runtimeClass match {
			case RamLike_ => Some(asInstanceOf[T])
			case _        => super.getInterface[T](tag)
		}

	}

	override def getRanges: Seq[(BigInt, BigInt)] = ranges
}

object RamInstance {
	def apply(ident: String,
	          definition: ChipDefinitionTrait,
	          attribs: Map[String, Variant]
	         ): Option[RamInstance] = {
		if ((!definition.attributes.contains("ranges")) && (!attribs.contains("ranges"))) {
			println(s"ERROR: ram ${ident} has no ranges, so isn't a valid range")
			return None
		}

		val ram = RamInstance(ident, definition)
		ram.mergeAllAttributes(attribs)
		Some(ram)
	}
}