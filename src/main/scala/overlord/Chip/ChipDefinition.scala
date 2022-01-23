package overlord.Chip

import ikuy_utils._
import overlord.{ChipDefinitionTrait, DefinitionType}

import java.nio.file.Path

trait ChipDefinition extends ChipDefinitionTrait {
	val defType   : DefinitionType
	val attributes: Map[String, Variant]
	val ports     : Map[String, Port]
	val registers : Option[Registers]
}

case class HardwareDefinition(defType: DefinitionType,
                              attributes: Map[String, Variant],
                              ports: Map[String, Port],
                              maxInstances: Int,
                              instanceAddressOffsets: Array[BigInt],
                              registers: Option[Registers])
	extends ChipDefinition

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

		val registers = if (table.contains("registers"))
			Some(Registers(Utils.toArray(table("registers")).toSeq, path))
		else None

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
			                   name.last,
			                   path.resolve(Utils.toString(gw)))

		} else {
			var mi  = 1
			var mao = Array[BigInt]()
			if (table.contains("hardware")) {
				val hw = Utils.toArray(table("hardware"))
				for (inlineTable <- hw) {
					val tab = inlineTable.asInstanceOf[TableV].value
					tab.keys.foreach {
						case f@("max_instances" | "max_cores") =>
							mi = Utils.toInt(tab(f))
						case f@("address_offsets")             =>
							mao = Utils.toArray(tab(f)).map(Utils.toBigInt)
						case _                                 =>
					}
				}
			}

			val (maxInstances, addressOffsets) = (mi, mao)
			Some(HardwareDefinition(defType,
			                        attribs,
			                        ports,
			                        maxInstances,
			                        addressOffsets,
			                        registers))
		}
	}
}