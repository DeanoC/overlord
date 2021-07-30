package overlord.Chip

import ikuy_utils._
import overlord.{ChipDefinitionTrait, DefinitionType}

import java.nio.file.Path

trait ChipDefinition extends ChipDefinitionTrait {
	val defType: DefinitionType
	val	attributes: Map[String, Variant]
	val	ports: Map[String, Port]
	val registers: Option[Registers]
}

case class HardwareDefinition(defType: DefinitionType,
                               attributes: Map[String, Variant],
                               ports: Map[String, Port],
                               registers: Option[Registers])
extends ChipDefinition

object ChipDefinition {
	def apply(table: Map[String, Variant], path: Path): ChipDefinitionTrait = {
		val gw = table.contains("gateware")

		val defTypeName = Utils.toString(table("type"))

		val attribs = table.filter(a => a._1 match {
			case "type" | "software" | "gateware" | "hardware" | "ports" | "registers" => false
			case _                                                                     => true
		})

		val name = defTypeName.split('.')

		val registers = if (table.contains("registers"))
			Some(Registers(Utils.toArray(table("registers")).toSeq, path))
		else None

		val ports = {
			if (table.contains("ports"))
				Ports(Utils.toArray(table("ports"))).map(t => t.name -> t).toMap
			else Map[String, Port]()
		}

		val defType = DefinitionType(defTypeName)

		if (gw) {
			GatewareDefinition(defType,
			                   attribs,
			                   ports,
			                   registers,
			                   name.last,
			                   path.resolve(Utils.toString(table("gateware")))).get

		} else {
			Some(HardwareDefinition(defType,
			               attribs,
			               ports,
			               registers)).get
		}
	}
}