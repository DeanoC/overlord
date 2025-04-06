package actions

import gagameos._
import input.VerilogPort
import overlord.Hardware.{Port, WireDirection}
import overlord.Project
import overlord.Instances.{ChipInstance, InstanceTrait}
import scala.util.boundary, boundary.break

case class ReadYamlRegistersAction(name: String, filename: String)
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
		val expandedName = Project.resolveInstanceMacros(instance, filename)
		val registers    = input.YamlRegistersParser(instance, expandedName, instance.name)
		instance.instanceRegisterBanks ++= registers
	}
}


object ReadYamlRegistersAction {
	def apply(name: String, process: Map[String, Variant]): Option[ReadYamlRegistersAction] = {
		if (!process.contains("source")) {
			println(s"Read Yaml Registers process $name doesn't have a source field")
			return None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
			case _          =>
				println("Read Yaml Register source field is malformed")
				return None
		}

		Some(ReadYamlRegistersAction(name, filename))
	}
}