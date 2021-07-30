package input

import ikuy_utils.{StringV, TableV}
import overlord.Chip.{RegisterBank, RegisterList, Registers}
import overlord.Software.Software

import java.nio.file.Path

object TomlRegistersParser {
	def apply(absolutePath: Path, name: String): Registers = {
		println(s"parsing $name toml for register definitions")
		val regSeq = Seq(TableV(Map{"resource" -> StringV(name)}))
		Registers(regSeq, absolutePath)
	}
}
