package overlord.Gateware.GatewareAction

import overlord.Gateware.Parameter

import java.nio.file.Path
import overlord.Instances.Instance
import overlord.Utils
import toml.Value

case class YamlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(gateware: Instance, parameters: Map[String, Parameter], outPath: Path): Unit = {
		val sb = new StringBuilder()
		for {k <- parameterKeys
		     if parameters.contains(k)} sb ++= {
			parameters(k).value match {
				case v: Value.Str  => s"$k: ${v.value}\n"
				case v: Value.Bool => s"$k: ${v.value}\n"
				case v: Value.Num  => s"$k: ${v.value}\n"
				case v: Value.Real => s"$k: ${v.value}\n"

				case _: Value.Arr  => assert(false); ""
				case _: Value.Tbl  => assert(false); ""
			}
		}

		Utils.ensureDirectories(outPath.resolve(filename).getParent)
		Utils.writeFile(outPath.resolve(filename), sb.result())

		updatePath(outPath)
	}
}

object YamlAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewareActionPathOp): Seq[YamlAction] = {
		if (!process.contains("parameters")) {
			println(s"Yaml process $name doesn't have a parameters field")
			None
		}
		if (!process("parameters").isInstanceOf[Value.Arr]) {
			println(s"Yaml process $name parameters isn't an array")
			None
		}
		if (!process.contains("filename")) {
			println(s"Yaml process $name doesn't have a filename field")
			None
		}
		if (!process("filename").isInstanceOf[Value.Str]) {
			println(s"Yaml process $name filename isn't a string")
			None
		}

		val filename   = Utils.toString(process("filename"))
		val parameters = Utils.toArray(process("parameters")).map(Utils.toString)

		Seq(YamlAction(parameters, filename, pathOp))
	}
}