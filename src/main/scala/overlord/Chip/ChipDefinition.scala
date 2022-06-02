package overlord.Chip

import ikuy_utils._
import overlord.{ChipDefinitionTrait, DefinitionType, HardwareDefinitionTrait}

import java.nio.file.Path

case class HardwareDefinition(defType: DefinitionType,
                              sourcePath: Path,
                              attributes: Map[String, Variant],
                              ports: Map[String, Port],
                              maxInstances: Int,
                              registersV: Seq[Variant])
	extends HardwareDefinitionTrait

object ChipDefinition {
	def apply(table: Map[String, Variant], path: Path): Option[ChipDefinitionTrait] = {
		val defTypeName = Utils.toString(table("type"))

		val attribs = table.filter(a => a._1 match {
			case "type" | "software" | "gateware" | "hardware" | "ports" | "registers" => false
			case _                                                                     => true
		})

		val name = defTypeName.split('.')
		if ((name(0) != "board" &&
		     name(0) != "pingroup" &&
		     name(0) != "other") && name.length <= 2) {
			println(s"$defTypeName must have at least 3 elements A.B.C")
			return None
		}

		val registers: Seq[Variant] = Utils.lookupArray(table, "registers").toSeq

		val ports = {
			if (table.contains("ports"))
				Ports(Utils.toArray(table("ports"))).map(t => t.name -> t).toMap
			else Map[String, Port]()
		}

		val defType = DefinitionType(defTypeName)

		if (table.contains("gateware")) {
			val gw = table("gateware")
			GatewareDefinition(defType,
			                   attribs,
			                   ports,
			                   registers,
			                   Utils.toString(gw))

		} else {
			val mi = Utils.lookupInt(table, "max_instances", 1)

			Some(HardwareDefinition(defType,
			                        path,
			                        attribs,
			                        ports,
			                        mi,
			                        registers))
		}
	}
}