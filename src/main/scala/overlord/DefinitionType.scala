package overlord

sealed trait DefinitionType {
	val ident: Seq[String]
}

case class RamDefinitionType(ident: Seq[String]) extends DefinitionType

case class CpuDefinitionType(ident: Seq[String]) extends DefinitionType

case class BusDefinitionType(ident: Seq[String]) extends DefinitionType

case class StorageDefinitionType(ident: Seq[String]) extends DefinitionType

case class BridgeDefinitionType(ident: Seq[String]) extends DefinitionType

case class NetDefinitionType(ident: Seq[String]) extends DefinitionType

case class OtherDefinitionType(ident: Seq[String]) extends DefinitionType

case class PinGroupDefinitionType(ident: Seq[String]) extends DefinitionType

case class ClockDefinitionType(ident: Seq[String]) extends DefinitionType

case class BoardDefinitionType(ident: Seq[String]) extends DefinitionType

case class ProgramDefinitionType(ident: Seq[String]) extends DefinitionType

case class LibraryDefinitionType(ident: Seq[String]) extends DefinitionType

object DefinitionType {
	def apply(in: String): DefinitionType = {
		val defTypeName = in.split('.')
		val tt          = defTypeName.tail.toSeq

		defTypeName.head.toLowerCase match {
			case "ram"      => RamDefinitionType(tt)
			case "cpu"      => CpuDefinitionType(tt)
			case "bus"      => BusDefinitionType(tt)
			case "storage"  => StorageDefinitionType(tt)
			case "bridge"   => BridgeDefinitionType(tt)
			case "net"      => NetDefinitionType(tt)
			case "board"    => BoardDefinitionType(tt)
			case "pin"      => PinGroupDefinitionType(tt)
			case "pingroup" => PinGroupDefinitionType(tt)
			case "clock"    => ClockDefinitionType(tt)
			case "other"    => OtherDefinitionType(tt)
			case "program"  => ProgramDefinitionType(tt)
			case "library"  => LibraryDefinitionType(tt)
			case _          =>
				println(s"${defTypeName.head} unknown definition type\n")
				OtherDefinitionType(tt)
		}
	}
}