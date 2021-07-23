package overlord.Hardware

import ikuy_utils._
import overlord.Definitions.HardwareTrait

case class Hardware(maxCount: Int) extends HardwareTrait

object Hardware {
	def apply(attribs: Seq[Variant]): Option[Hardware] = {
		var maxCount = 1
		for( table <- attribs) {
			maxCount = Utils.lookupInt(Utils.toTable(table), "max_cores", 1)
		}
		Some(Hardware(maxCount))
	}
}