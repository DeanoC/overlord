package overlord.Instances

import gagameos.Variant
import overlord.SoftwareDefinitionTrait

case class ProgramInstance(
    name: String,
    override val definition: SoftwareDefinitionTrait
) extends SoftwareInstance {
  override val folder = "programs"
}

object ProgramInstance {
  def apply(
      ident: String,
      definition: SoftwareDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, ProgramInstance] = {

    if (
      !attribs.contains("name") &&
      !definition.attributes.contains("name")
    ) {
      Left(f"Programs must have a name attribute")
    } else {
      val sw = ProgramInstance(ident, definition)
      sw.mergeAllAttributes(attribs)
      Right(sw)
    }
  }
}
