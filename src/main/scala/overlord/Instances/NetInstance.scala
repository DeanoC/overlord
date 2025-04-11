package overlord.Instances

import gagameos.Variant
import overlord.ChipDefinitionTrait

case class NetInstance(
    name: String,
    override val definition: ChipDefinitionTrait
) extends ChipInstance {
  override def isVisibleToSoftware: Boolean = true
}

object NetInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, NetInstance] = {
    val net = NetInstance(ident, definition)
    net.mergeAllAttributes(attribs)
    Right(net)
  }
}
