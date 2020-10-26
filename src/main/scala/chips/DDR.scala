package overlord.chips
import overlord._

case class DDR3(override val size : String) extends RamChip("DDR3", true, size)
case class DDR4(override val size : String) extends RamChip("DDR4", true, size)
