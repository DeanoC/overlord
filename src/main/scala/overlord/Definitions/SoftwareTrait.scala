package overlord.Definitions

import overlord.Software.{Register, RegisterBank}

trait SoftwareTrait {
	val groups: Seq[SoftwareGroup]
}

case class SoftwareGroup(description: String,
                         banks: Array[RegisterBank],
                         registers: Array[Register],
                        ) {
	def bankSize: BigInt = registers.last.offset
}

