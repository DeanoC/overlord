package overlord.Definitions

import overlord.Software.{Register, RegisterBank}

trait SoftwareTrait {
	val groups: Seq[SoftwareGroup]
}

case class SoftwareGroup(val description: String,
                         val banks: Array[RegisterBank],
                         val registers: Array[Register],
                        ) {
	def bankSize: String = registers.last.offset
}

