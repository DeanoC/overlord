package overlord.Definitions

import ikuy_utils.Variant
import overlord.Gateware.Port
import overlord.Instances._
import overlord.Software.{RegisterBank, RegisterList}
import toml.Value


sealed trait DefinitionType {
	val ident: Seq[String]
}

case class RamDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class CpuDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class BusDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class StorageDefinitionType(ident: Seq[String]
                                ) extends DefinitionType

case class SocDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class BridgeDefinitionType(ident: Seq[String]
                               ) extends DefinitionType

case class NetDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class OtherDefinitionType(ident: Seq[String]
                              ) extends DefinitionType

case class PinGroupDefinitionType(ident: Seq[String]
                                 ) extends DefinitionType

case class ClockDefinitionType(ident: Seq[String])
	extends DefinitionType

case class BoardDefinitionType(ident: Seq[String]
                              ) extends DefinitionType


trait DefinitionTrait {
	val defType   : DefinitionType
	val ports     : Map[String, Port]
	val attributes: Map[String, Variant]

	val gateware  : Option[GatewareTrait]
	val hardware  : Option[HardwareTrait]

	val registerBanks: Seq[RegisterBank]
	val registerLists: Seq[RegisterList]
	val docs         : Seq[String]

	def createInstance(name: String,
	                   attribs: Map[String, Variant]
	                  ): Option[Instance] = {
		defType match {
			case _: RamDefinitionType      => RamInstance(name, this, attribs)
			case _: CpuDefinitionType      => CpuInstance(name, this, attribs)
			case _: BusDefinitionType      => BusInstance(name, this, attribs)
			case _: StorageDefinitionType  => StorageInstance(name, this, attribs)
			case _: SocDefinitionType      => SocInstance(name, this, attribs)
			case _: BridgeDefinitionType   => BridgeInstance(name, this, attribs)
			case _: NetDefinitionType      => NetInstance(name, this, attribs)
			case _: OtherDefinitionType    => OtherInstance(name, this, attribs)
			case _: PinGroupDefinitionType => PinGroupInstance(name, this, attribs)
			case _: ClockDefinitionType    => ClockInstance(name, this, attribs)
			case _: BoardDefinitionType    => BoardInstance(name, this, attribs)
		}
	}
}

