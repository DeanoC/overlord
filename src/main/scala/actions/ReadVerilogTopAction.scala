package actions

import ikuy_utils._
import input.{VerilogParameterKey, VerilogPort}
import overlord.Chip.{Port, WireDirection}
import overlord.Game
import overlord.Instances.{ChipInstance, InstanceTrait}

case class ReadVerilogTopAction(filename: String)
	extends GatewareAction {

	override val phase: Int = 2

	def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		if (!instance.isInstanceOf[ChipInstance]) {
			println(s"${instance.name} is not a chip but is being processed by a gateware action")
		} else {
			execute(instance.asInstanceOf[ChipInstance], parameters)
		}
	}

	override def execute(instance: ChipInstance, parameters: Map[String, Variant]): Unit = {
		val expandedName = Game.resolveInstanceMacros(instance, instance.name)
		val bs           = input.VerilogModuleParser(
			Game.tryPaths(instance, expandedName + ".v"),
			expandedName.split("/").last)

		bs.filter(_.isInstanceOf[VerilogPort])
			.map(_.asInstanceOf[VerilogPort])
			.foreach(p => instance.mergePort(p.name, Port(p.name, p.bits, WireDirection(p.direction))))

		bs.filter(_.isInstanceOf[VerilogParameterKey])
			.map(_.asInstanceOf[VerilogParameterKey])
			.foreach(p => instance.mergeParameterKey(p.parameter))
	}
}

object ReadVerilogTopAction {
	def apply(name: String,
	          process: Map[String, Variant]): Option[ReadVerilogTopAction] = {
		if (!process.contains("source")) {
			println(s"Read Verilog Top process $name doesn't have a source field")
			return None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
			case _          => println("Source is an invalid type"); ""
		}

		Some(ReadVerilogTopAction(filename))
	}
}
