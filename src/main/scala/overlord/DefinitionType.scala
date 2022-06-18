package overlord

sealed trait DefinitionType {
	val ident: Seq[String]
}

sealed trait ChipDefinitionType extends DefinitionType

sealed trait PortDefinitionType extends DefinitionType

sealed trait SoftwareDefinitionType extends DefinitionType

case class RamDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class CpuDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class GraphicDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class StorageDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class NetDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class IoDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class OtherDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class SocDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class SwitchDefinitionType(ident: Seq[String]) extends ChipDefinitionType

case class BoardDefinitionType(ident: Seq[String]) extends DefinitionType

case class PinGroupDefinitionType(ident: Seq[String]) extends PortDefinitionType

case class ClockDefinitionType(ident: Seq[String]) extends PortDefinitionType

case class ProgramDefinitionType(ident: Seq[String]) extends SoftwareDefinitionType

case class LibraryDefinitionType(ident: Seq[String]) extends SoftwareDefinitionType

object DefinitionType {
	def apply(in: String): DefinitionType = {
		val defTypeName = in.split('.')
		val tt          = defTypeName.map {
			_.toLowerCase
		}.toSeq

		defTypeName.head.toLowerCase match {
			case "ram"     => RamDefinitionType(tt)
			case "cpu"     => CpuDefinitionType(tt)
			case "storage" => StorageDefinitionType(tt)
			case "graphic" => GraphicDefinitionType(tt)
			case "net"     => NetDefinitionType(tt)
			case "io"      => IoDefinitionType(tt)
			case "board"   => BoardDefinitionType(tt)
			case "soc"     => SocDefinitionType(tt)
			case "switch"  => SwitchDefinitionType(tt)
			case "other"   => OtherDefinitionType(tt)

			case "pin"      => PinGroupDefinitionType(tt)
			case "pingroup" => PinGroupDefinitionType(tt)
			case "clock"    => ClockDefinitionType(tt)

			case "program" => ProgramDefinitionType(tt)
			case "library" => LibraryDefinitionType(tt)
			case _         =>
				println(s"${defTypeName.head} unknown definition type\n")
				OtherDefinitionType(tt)
		}
	}
}