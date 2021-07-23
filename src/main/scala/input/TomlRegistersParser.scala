package input

import ikuy_utils.{StringV, TableV}
import overlord.Software.Software

import java.nio.file.Path

object TomlRegistersParser {
	def apply(absolutePath: Path, name: String): Option[Software] = {
		println(s"parsing $name toml for register definitions")
		val regSeq = Seq(TableV(Map{"registers" -> StringV(name)}))
		Software(regSeq, absolutePath)
	}

}
