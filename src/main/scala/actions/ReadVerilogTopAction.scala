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
		val expandedName = Game.resolveInstanceMacros(instance, filename)
		val modules      = input.VerilogModuleParser(Game.tryPaths(instance, expandedName), instance.name)
		val module       = modules.find(_.name == instance.name).getOrElse {
			modules.find(_.name == expandedName.split("/").last.replace(".v", "")).getOrElse(
				return
				)
		}

		instance.moduleName = module.name
		val ports         = module.boundary.collect { case p: VerilogPort => p }
		val parameterKeys = module.boundary.collect { case p: VerilogParameterKey => p }
		ports.foreach(p => instance.mergePort(p.name, Port(p.name, p.bits, WireDirection(p.direction), p.knownWidth)))
		parameterKeys.foreach(p => instance.mergeParameterKey(p.parameter))

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
