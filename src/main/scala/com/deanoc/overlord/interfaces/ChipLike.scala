package com.deanoc.overlord.Interfaces

import com.deanoc.overlord.QueryInterface
import com.deanoc.overlord.Instances.ChipInstance

trait ChipLike extends QueryInterface {
  def getOwner: ChipInstance
}
