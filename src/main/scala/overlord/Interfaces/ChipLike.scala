package overlord.Interfaces

import overlord.QueryInterface
import overlord.Instances.ChipInstance

trait ChipLike extends QueryInterface {
  def getOwner: ChipInstance
}
