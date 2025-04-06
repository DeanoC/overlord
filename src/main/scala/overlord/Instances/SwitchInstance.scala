package overlord.Instances

import gagameos.Variant
import overlord.ChipDefinitionTrait

case class SwitchInstance(name: String, definition: ChipDefinitionTrait)
    extends ChipInstance {}

object SwitchInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Option[SwitchInstance] = {
    val chip = SwitchInstance(ident, definition)
    chip.mergeAllAttributes(attribs)
    Some(chip)
  }
}
