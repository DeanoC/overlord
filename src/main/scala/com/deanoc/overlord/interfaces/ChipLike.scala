package com.deanoc.overlord.interfaces

import com.deanoc.overlord.QueryInterface
import com.deanoc.overlord.instances.ChipInstance

trait ChipLike extends QueryInterface {
  def getOwner: ChipInstance
}
