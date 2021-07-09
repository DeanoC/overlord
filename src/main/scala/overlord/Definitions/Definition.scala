package overlord.Definitions

import overlord.Gateware.{Gateware, Port, Ports}
import overlord.Software.{RegisterBank, RegisterList, Software}
import ikuy_utils._

import java.nio.file.Path

case class Definition(defType: DefinitionType,
                      attributes: Map[String, Variant],
                      ports: Map[String, Port] = Map[String, Port](),
                      parameters: Map[String, Variant] = Map[String, Variant](),
                      gateware: Option[GatewareTrait] = None,
                      hardware: Option[HardwareTrait] = None,
                      registerBanks: Seq[RegisterBank] = Seq[RegisterBank](),
                      registerLists: Seq[RegisterList] = Seq[RegisterList](),
                      docs: Seq[String] = Seq[String]()
                     )
	extends DefinitionTrait

object Definition {
	def toDefinitionType(in: String): DefinitionType = {
		val defTypeName = in.split('.')
		val tt          = defTypeName.tail.toSeq

		defTypeName.head.toLowerCase match {
			case "ram"     => RamDefinitionType(tt)
			case "cpu"     => CpuDefinitionType(tt)
			case "bus"     => BusDefinitionType(tt)
			case "storage" => StorageDefinitionType(tt)
			case "soc"     => SocDefinitionType(tt)
			case "bridge"  => BridgeDefinitionType(tt)
			case "net"     => NetDefinitionType(tt)
			case "board"   => BoardDefinitionType(tt)
			case "pin"     => PinGroupDefinitionType(tt)
			case "clock"   => ClockDefinitionType(tt)
			case _         => OtherDefinitionType(tt)
		}
	}

	def apply(chip: Variant, path: Path): Definition = {
		val table       = Utils.toTable(chip)
		val defTypeName = Utils.toString(table("type"))

		val attribs = table.filter(a => a._1 match {
			case "type" | "software" |
			     "gateware" | "hardware" => false
			case _                       => true
		})

		val sw = if (table.contains("software"))
			Software(Utils.toArray(table("software")).toSeq, path)
		else None

		val name = defTypeName.split('.')

		val gw = if (table.contains("gateware"))
			Gateware(name.last,
			         path.resolve(Utils.toString(table("gateware"))))
		else None

		val hw = None

		val ports = {
			if (table.contains("ports"))
				Ports(Utils.toArray(table("ports")))
					.map(t => t.name -> t).toMap
			else Map[String, Port]()
		}

		val parameters = if (table.contains("parameters"))
			Utils.toTable(table("parameters"))
		else Map[String, Variant]()

		Definition(toDefinitionType(defTypeName),
		           attribs,
		           ports,
		           parameters,
		           gw, hw,
		           if(sw.nonEmpty) sw.get.banks else Seq(),
		           if(sw.nonEmpty) sw.get.registerLists else Seq(),
		           if(sw.nonEmpty) sw.get.docs else Seq())
	}
}