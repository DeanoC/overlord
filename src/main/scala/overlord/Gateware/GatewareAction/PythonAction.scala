package overlord.Gateware.GatewareAction

import java.nio.file.Path

import overlord.Gateware.Gateware
import overlord.Instances.Instance
import overlord.Utils
import toml.Value

case class PythonAction(script: String,
                        args: String,
                        pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(instance: Instance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		import scala.language.postfixOps

		val result = sys.process.Process(
			Seq("python3", s"$script", s"$args"), outPath.toFile).!

		if (result != 0)
			println(s"FAILED python3 of $script $args")

	}
}

object PythonAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewarePathOp): Seq[PythonAction] = {
		if (!process.contains("script")) {
			println(s"Python process $name doesn't have a script field")
			None
		}
		if (!process("script").isInstanceOf[Value.Str]) {
			println(s"Python process $name script isn't a string")
			None
		}
		if (!process.contains("args")) {
			println(s"Python process $name doesn't have a args field")
			None
		}
		if (!process("args").isInstanceOf[Value.Str]) {
			println(s"Python process $name args isn't a string")
			None
		}
		val script = Utils.toString(process("script"))
		val args   = Utils.toString(process("args"))
		Seq(PythonAction(script, args, pathOp))
	}
}