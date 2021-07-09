package overlord.Gateware.GatewareAction

import input.{VerilogParameterKey, VerilogPort}
import overlord.Gateware.{Port, WireDirection}
import overlord.Instances.Instance
import overlord.Game
import ikuy_utils._

import java.nio.file.Path

case class ReadVerilogTopAction(filename: String,
                                pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase2()

	override def execute(instance: Instance,
	                     parameters: Map[String, Variant],
	                     outPath: Path): Unit = {
		val name = filename.replace("${name}", instance.ident)
		val bs   = input.VerilogModuleParser(
			outPath.resolve(name + ".v"), name.split("/").last)

		if (instance.isGateware) {
			instance.instancePorts ++=
			bs.filter(_.isInstanceOf[VerilogPort])
				.map(_.asInstanceOf[VerilogPort])
				.map(p => p.name -> Port(p.name, p.bits, WireDirection(p.direction)))

			instance.instanceParameterKeys ++=
			bs.filter(_.isInstanceOf[VerilogParameterKey])
				.map(_.asInstanceOf[VerilogParameterKey])
				.map(p => p.parameter)
		}

		updatePath(outPath)
	}
}

object ReadVerilogTopAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: GatewareActionPathOp): Option[ReadVerilogTopAction] = {
		if (!process.contains("source")) {
			println(s"Read Verilog Top process $name doesn't have a source field")
			None
		}

		val filename = process("source") match {
			case s: StringV => s.value
			case t: TableV  => Utils.toString(t.value("file"))
		}

		val srcPath = if (filename.contains("${src}")) {
			val tmp = filename.replace("${src}", "")
			Game.pathStack.top.toString + tmp
		} else filename
		Some(ReadVerilogTopAction(srcPath, pathOp))
	}
}
