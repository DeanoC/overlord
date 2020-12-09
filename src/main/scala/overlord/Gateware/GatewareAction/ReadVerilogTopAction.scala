package overlord.Gateware.GatewareAction

import input.{VerilogParameter, VerilogPort}
import overlord.Gateware.{BitsDesc, Parameter, Port, WireDirection}
import overlord.Instances.Instance
import overlord.{Game, Utils}
import toml.Value

import java.nio.file.Path

case class ReadVerilogTopAction(filename: String,
                                name: String,
                                pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase2()

	override def execute(gateware: Instance,
	                     parameters: Map[String, Parameter],
	                     outPath: Path): Unit = {
		val bs = input.VerilogModuleParser(outPath.resolve(filename), name)

		if (gateware.isGateware) {
			gateware.definition.gateware.get.ports ++=
			bs.filter(_.isInstanceOf[VerilogPort])
				.map(_.asInstanceOf[VerilogPort])
				.map(p => (p.name -> Port(p.name, p.bits, WireDirection(p.direction))))

			gateware.definition.gateware.get.verilog_parameters ++=
			bs.filter(_.isInstanceOf[VerilogParameter])
				.map(_.asInstanceOf[VerilogParameter])
				.map(p => p.parameter)
		}

		updatePath(outPath)
	}
}

object ReadVerilogTopAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewareActionPathOp): Option[ReadVerilogTopAction] = {
		if (!process.contains("source")) {
			println(s"Read Verilog Top process $name doesn't have a source field")
			None
		}
		val src = Utils.toTable(process("source"))

		val filename = Utils.toString(src("file"))
		val srcPath  = if (filename.contains("${src}")) {
			val tmp = filename.replace("${src}", "")
			Game.pathStack.top.toString + tmp
		} else filename

		val moduleName = Utils.lookupString(src, "module_name", "top")
		Some(ReadVerilogTopAction(srcPath, moduleName, pathOp))
	}
}
