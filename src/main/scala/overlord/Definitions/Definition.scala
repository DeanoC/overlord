package overlord.Definitions

import overlord.Gateware.Gateware
import overlord.Software.Software
import overlord.Utils
import toml.Value

import java.nio.file.Path

case class Definition(defType: DefinitionType,
                      attributes: Map[String, Value] = Map[String, Value](),
                      software: Option[SoftwareTrait] = None,
                      gateware: Option[GatewareTrait] = None,
                      hardware: Option[HardwareTrait] = None)
	extends DefinitionTrait

object Definition {
	def toDefinitionType(in: String,
	                     ports: Seq[String]): DefinitionType = {
		val defTypeName = in.split('.')

		defTypeName.head match {
			case "ram"             =>
				RamDefinitionType(defTypeName.tail, ports)
			case "cpu"             =>
				CpuDefinitionType(defTypeName.tail, ports)
			case "NxMinterconnect" =>
				NxMDefinitionType(defTypeName.tail, ports)
			case "storage"         =>
				StorageDefinitionType(defTypeName.tail, ports)
			case "soc"             =>
				SocDefinitionType(defTypeName.tail, ports)
			case "bridge"          =>
				BridgeDefinitionType(defTypeName.tail, ports)
			case "net"             =>
				NetDefinitionType(defTypeName.tail, ports)
			case _                 =>
				OtherDefinitionType(defTypeName.tail, ports)
		}
	}

	def apply(chip: Value, registerPath: Path): Definition = {

		val table       = Utils.toTable(chip)
		val defTypeName = Utils.toString(table("type"))

		val attribs = table.filter(a => a._1 match {
			case "type" | "software" | "gateware" | "hardware" => false
			case _                                             => true
		})

		val sw = if (table.contains("software"))
			Software(Utils.toArray(table("software")), registerPath)
		else None

		val name = defTypeName.split('.')

		val gw = if (table.contains("gateware"))
			Gateware(name.last,
			         registerPath.resolve(Path.of(
				         s"${name.last}/${name.last}" + s".toml")))
		else None

		val hw = None

		val ports = if (gw.nonEmpty) gw.get.ports
		else Seq[String]()

		val parameters = if (gw.nonEmpty) gw.get.parameters.map(_._1).toSeq
		else Seq[String]()

		val defType = toDefinitionType(defTypeName, ports)

		Definition(defType, attribs, sw, gw, hw)

	}
}