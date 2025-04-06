package overlord

import overlord.Hardware.Port
import overlord.Instances.ChipInstance

trait PortsLike extends ChipLike {
  def getPortsStartingWith(startsWith: String): Seq[Port]
  def getPortsMatchingName(name: String): Seq[Port]
}