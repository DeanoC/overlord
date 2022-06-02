package input

import ikuy_utils.{StringV, TableV}
import overlord.Chip.{RegisterBank, Registers}
import overlord.Instances.InstanceTrait

object TomlRegistersParser {
	def apply(instance: InstanceTrait, name: String): Seq[RegisterBank] = {
		println(s"parsing $name toml for register definitions")
		val regSeq = Seq(TableV(Map {
			"resource" -> StringV(name)
		}))
		Registers(instance, regSeq)
	}
}
