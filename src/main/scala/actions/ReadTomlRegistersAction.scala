package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.{ChipInstance, InstanceTrait}

case class ReadTomlRegistersAction(filename: String)
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
		val name      = Game.resolvePathMacros(instance, instance.name)
		val registers = input.TomlRegistersParser(instance, name.split("/").last)
		instance.instanceRegisterBanks ++= registers
	}
}

object ReadTomlRegistersAction {
	def apply(name: String, process: Map[String, Variant]): Option[ReadTomlRegistersAction] = {
		if (!process.contains("source")) {
			println(s"Read Toml Registers process $name doesn't have a source field")
			return None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
			case _          =>
				println("Read Toml Register source field is malformed")
				return None
		}

		Some(ReadTomlRegistersAction(filename))
	}
}
