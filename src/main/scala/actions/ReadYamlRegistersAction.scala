package actions

import gagameos._
import input.VerilogPort
import overlord.Chip.{Port, WireDirection}
import overlord.Project
import overlord.Instances.{ChipInstance, InstanceTrait}
import scala.util.boundary, boundary.break

case class ReadYamlRegistersAction(name: String, process: Map[String, Variant])
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
		import scala.util.boundary, boundary.break

		val expandedName = Project.resolveInstanceMacros(instance, name)
		val registers    = input.YamlRegistersParser(instance, expandedName, instance.name)

		boundary {
			// ...existing code...
		}
	}
}