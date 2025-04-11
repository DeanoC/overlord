package overlord.Instances

import gagameos.Variant
import overlord.SoftwareDefinitionTrait

case class LibraryInstance(
    override val name: String,
    override val definition: SoftwareDefinitionTrait
) extends SoftwareInstance {
  override val folder = "libs"
}

object LibraryInstance {
  def apply(
      ident: String,
      definition: SoftwareDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, LibraryInstance] = {

    if (
      !attribs.contains("name") &&
      !definition.attributes.contains("name")
    ) {
      Left(f"Libraries must have a name attribute")
    } else {
      val sw = LibraryInstance(ident, definition)
      sw.mergeAllAttributes(attribs)
      Right(sw)
    }
  }
}
