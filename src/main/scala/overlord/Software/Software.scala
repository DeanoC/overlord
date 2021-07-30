package overlord.Software

import ikuy_utils.Variant
import overlord.Chip.{RegisterBank, RegisterList, Registers}

import java.nio.file.Path

case class Software()

object Software {
	def apply(attribs: Seq[Variant], path: Path): Software = {

		val registers = Registers(attribs, path)

		Software()
	}
}