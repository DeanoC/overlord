package overlord

import scala.collection.mutable.ArrayBuffer

class Chip(  override val name : String, 
                      override val isHard : Boolean,
                      val chipType : String ) 
extends BoardFeature(name, isHard)

class RamChip(  override val name : String, 
                override val isHard : Boolean, 
                val size : String ) 
extends Chip(name, isHard, "Ram")

class CpuClusterChip( override val name : String, override val isHard : Boolean,
                      val coreCount : Int, 
                      val arch : String, 
                      val width : Int,
                      val localRam : Option[String])
extends Chip(name, isHard, "CpuCluster")

