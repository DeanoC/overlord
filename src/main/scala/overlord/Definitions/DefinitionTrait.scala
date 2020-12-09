package overlord.Definitions

import overlord.Gateware.{Parameter, Port}
import overlord.Instances._
import toml.Value


sealed trait DefinitionType {
	val ident: Seq[String]
}

case class RamDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class CpuDefinitionType(ident: Seq[String]
                            ) extends DefinitionType

case class NxMDefinitionType(ident: Seq[String]
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

case class ConstantDefinitionType(ident: Seq[String]
                                 ) extends DefinitionType

case class BoardDefinitionType(ident: Seq[String]
                              ) extends DefinitionType


trait DefinitionTrait {
	val defType   : DefinitionType
	val ports     : Map[String, Port]
	val parameters: Map[String, Parameter]
	val attributes: Map[String, toml.Value]
	val software  : Option[SoftwareTrait]
	val gateware  : Option[GatewareTrait]
	val hardware  : Option[HardwareTrait]

	def createInstance(name: String,
	                   attribs: Map[String, Value]
	                  ): Option[Instance] = {
		defType match {
			case _: RamDefinitionType      =>
				Some(RamInstance(name, this, attribs))
			case _: CpuDefinitionType      =>
				Some(CpuInstance(name, this, attribs))
			case _: NxMDefinitionType      =>
				Some(NxMInstance(name, this, attribs))
			case _: StorageDefinitionType  =>
				Some(StorageInstance(name, this, attribs))
			case _: SocDefinitionType      =>
				Some(SocInstance(name, this, attribs))
			case _: BridgeDefinitionType   =>
				Some(BridgeInstance(name, this, attribs))
			case _: NetDefinitionType      =>
				Some(NetInstance(name, this, attribs))
			case _: OtherDefinitionType    =>
				Some(OtherInstance(name, this, attribs))
			case _: PinGroupDefinitionType =>
				PinGroupInstance(name, this, attribs)
			case _: ClockDefinitionType    =>
				ClockInstance(name, this, attribs)

			case _: ConstantDefinitionType => ???
			case _: BoardDefinitionType    =>
				BoardInstance(name = name, this, attribs)
		}
	}
}

