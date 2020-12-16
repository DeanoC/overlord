package overlord.Gateware.GatewareAction

import java.nio.file.Path
import overlord.Gateware.Parameter
import overlord.Instances.Instance
import ikuy_utils._
import toml.Value

case class PythonAction(script: String,
                        args: String,
                        pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(instance: Instance, parameters: Map[String, Parameter], outPath: Path): Unit = {
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
	          pathOp: GatewareActionPathOp): Seq[PythonAction] = {
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