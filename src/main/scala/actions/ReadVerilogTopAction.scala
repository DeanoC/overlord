package actions

import ikuy_utils._
import input.{VerilogParameterKey, VerilogPort}
import overlord.Chip.{Port, WireDirection}
import overlord.Game
import overlord.Instances.{ChipInstance, InstanceTrait}

import java.nio.file.Path

case class ReadVerilogTopAction(filename: String,
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
		val name = filename.replace("${name}", instance.ident)
		val bs   = input.VerilogModuleParser(
			outPath.resolve(name + ".v"), name.split("/").last)

		bs.filter(_.isInstanceOf[VerilogPort])
			.map(_.asInstanceOf[VerilogPort])
			.foreach(p => instance.mergePort(p.name,
			                                 Port(p.name,
			                                      p.bits,
			                                      WireDirection(p.direction))))

		bs.filter(_.isInstanceOf[VerilogParameterKey])
			.map(_.asInstanceOf[VerilogParameterKey])
			.foreach(p => instance.mergeParameterKey(p.parameter))

		updatePath(outPath)
	}
}

object ReadVerilogTopAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Option[ReadVerilogTopAction] = {
		if (!process.contains("source")) {
			println(s"Read Verilog Top process $name doesn't have a source field")
			return None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
			case _          => println("Source is an invalid type"); ""
		}

		val srcPath = if (filename.contains("${src}")) {
			val tmp = filename.replace("${src}", "")
			Game.pathStack.top.toString + tmp
		} else filename
		Some(ReadVerilogTopAction(srcPath, pathOp))
	}
}
