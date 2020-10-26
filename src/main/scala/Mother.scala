package overlord

import scala.collection.immutable.HashMap

class Mother( val board : Board, val chipLibraries : Array[Chip] )
{
  var ramChips = HashMap[String, RamChip]()
  var cpuClusterChips = HashMap[String, CpuClusterChip]()
  var otherChips = HashMap[String, Chip]()

  var buses = HashMap[String, Bus]()

  private def matchChips(chip : Chip) : Unit = {
    chip match {
      case cc: CpuClusterChip => cpuClusterChips += (cc.name -> cc)
      case rc: RamChip => ramChips += (rc.name -> rc)
      case other => otherChips += (other.name -> other)
    }
  } 

  def hasChip(name : String ) : Boolean = ramChips.contains(name) || cpuClusterChips.contains(name) || otherChips.contains(name)

  def getChip(name : String) : Chip = {
    assert(hasChip(name))

    if(ramChips.contains(name)) ramChips(name)
    else if(cpuClusterChips.contains(name)) cpuClusterChips(name)
    else if(otherChips.contains(name)) otherChips(name)
    else null // should never happen
  }

  for(chip <- board.chips) matchChips(chip)

  for(chip <- chipLibraries) matchChips(chip)
  

}