package overlord.Definitions


sealed trait DefinitionType {
	val ident: Seq[String]

	val portsOrParameters: Seq[String]

	def hasPortsOrParameters: Boolean = portsOrParameters.nonEmpty

}

case class RamDefinitionType(
	                            ident: Seq[String],
	                            portsOrParameters: Seq[String]
                            ) extends DefinitionType

case class CpuDefinitionType(
	                            ident: Seq[String],
	                            portsOrParameters: Seq[String]) extends DefinitionType

case class NxMDefinitionType(
	                            ident: Seq[String],
	                            portsOrParameters: Seq[String]) extends DefinitionType

case class StorageDefinitionType(
	                                ident: Seq[String],
	                                portsOrParameters: Seq[String])
	extends DefinitionType

case class SocDefinitionType(
	                            ident: Seq[String],
	                            portsOrParameters: Seq[String]) extends DefinitionType

case class BridgeDefinitionType(
	                               ident: Seq[String],
	                               portsOrParameters: Seq[String]) extends
	DefinitionType

case class NetDefinitionType(
	                            ident: Seq[String],
	                            portsOrParameters: Seq[String]) extends DefinitionType

case class OtherDefinitionType(
	                              ident: Seq[String],
	                              portsOrParameters: Seq[String]) extends DefinitionType

case class PortDefinitionType(
	                             ident: Seq[String],
	                             portsOrParameters: Seq[String]) extends DefinitionType

case class ClockDefinitionType(
	                              ident: Seq[String],
	                              portsOrParameters: Seq[String]) extends DefinitionType

case class ConstantDefinitionType(
	                                 ident: Seq[String],
	                                 portsOrParameters: Seq[String])
	extends DefinitionType

trait DefinitionTrait {
	val defType   : DefinitionType
	val attributes: Map[String, toml.Value]
	val software  : Option[SoftwareTrait]
	val gateware  : Option[GatewareTrait]
	val hardware  : Option[HardwareTrait]
}


