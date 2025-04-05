package input

import gagameos.{BigIntV, StringV, TableV}
import overlord.Chip.{RegisterBank, Registers}
import overlord.Instances.InstanceTrait

object TomlRegistersParser {
	def apply(instance: InstanceTrait, filename: String, name: String): Seq[RegisterBank] = {
		println(s"parsing $name toml for register definitions")
		val regSeq = Seq(TableV(Map(
			"resource" -> StringV(filename),
			"name" -> StringV(name),
			"base_address" -> BigIntV(0),
			)))
		Registers(instance, regSeq)
	}
}
