package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.{ChipInstance, InstanceTrait}

import java.nio.file.Path

case class ReadTomlRegistersAction(filename: String,
                                   pathOp: ActionPathOp)
	extends GatewareAction {

	override val phase: Int = 2

	def execute(instance: InstanceTrait,
	            parameters: Map[String, () => Variant],
	            outPath: Path): Unit = {
		if (!instance.isInstanceOf[ChipInstance]) {
			println(s"${
				instance
					.ident
			} is not a chip but is being processed by a gateware action")
		} else {
			execute(instance.asInstanceOf[ChipInstance], parameters, outPath)
		}
	}

	override def execute(instance: ChipInstance,
	                     parameters: Map[String, () => Variant],
	                     outPath: Path): Unit = {
		val name      = filename.replace("${name}", instance.ident)
		val registers = input.TomlRegistersParser(outPath, name.split("/").last)
		instance.instanceRegisterBanks ++= registers.banks
		instance.instanceRegisterLists ++= registers.lists
	}
}

object ReadTomlRegistersAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Option[ReadTomlRegistersAction] = {
		if (!process.contains("source")) {
			println(s"Read Toml Registers process $name doesn't have a source field")
			return None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
			case _ =>
				println("Read Toml Register source field is malformed")
				return None
		}

		val srcPath = if (filename.contains("${src}")) {
			val tmp = filename.replace("${src}", "")
			Game.pathStack.top.toString + tmp
		} else filename
		Some(ReadTomlRegistersAction(srcPath, pathOp))
	}
}
