package overlord.Connections

import overlord._
import overlord.DefinitionType
import overlord.Instances.ChipInstance

trait ConnectedBetween extends Connected {
  val connectionPriority: ConnectionPriority
  val direction: ConnectionDirection

  def mainType: Option[DefinitionType] =
    if (first.nonEmpty) Some(first.get.definition.defType) else None

  def secondaryType: Option[DefinitionType] =
    if (second.nonEmpty) Some(second.get.definition.defType) else None

  override def connectedTo(inst: ChipInstance): Boolean =
    (first.nonEmpty && first.get.instance.name == inst.name) || (second.nonEmpty && second.get.instance.name == inst.name)

  override def connectedBetween(
      s: ChipInstance,
      e: ChipInstance,
      d: ConnectionDirection
  ): Boolean = {
    if (first.isEmpty || second.isEmpty) false
    else {
      d match {
        case FirstToSecondConnection() => (
          first.get.instance == s && second.get.instance == e
        )
        case SecondToFirstConnection() => (
          first.get.instance == e && second.get.instance == s
        )
        case BiDirectionConnection() => (
          (first.get.instance == s && second.get.instance == e) || (first.get.instance == e && second.get.instance == s)
        )
      }
    }
  }

  override def isPinToChip: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isPin && second.get.isChip

  override def isChipToChip: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isChip && second.get.isChip

  override def isChipToPin: Boolean =
    first.nonEmpty && second.nonEmpty && first.get.isChip && second.get.isPin

  override def isClock: Boolean =
    (first.nonEmpty && first.get.isClock) || (second.nonEmpty && second.get.isClock)

  override def firstFullName: String =
    if (first.nonEmpty) first.get.fullName else "NOT_CONNECTED"

  override def secondFullName: String =
    if (second.nonEmpty) second.get.fullName else "NOT_CONNECTED"

}
