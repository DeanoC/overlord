package overlord

import overlord.Instances.ChipInstance

trait ChipLike extends QueryInterface {
  def getOwner: ChipInstance
}