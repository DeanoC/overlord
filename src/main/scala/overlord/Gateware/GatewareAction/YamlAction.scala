package overlord.Gateware.GatewareAction

import java.nio.file.Path

import overlord.Instances.Instance
import overlord.Utils
import toml.Value

case class YamlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(gateware: Instance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		val sb = new StringBuilder()
		for (k <- parameterKeys) {
			if (parameters.contains(k))
				sb ++= s"$k: ${parameters(k)}\n"
		}
		Utils.writeFile(outPath.resolve(filename), sb.result())

		updatePath(outPath)
	}
}

object YamlAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewarePathOp): Seq[YamlAction] = {
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