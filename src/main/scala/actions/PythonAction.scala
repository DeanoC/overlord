package actions

import ikuy_utils._
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class PythonAction(script: String,
                        args: String,
                        pathOp: ActionPathOp)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait,
	                     parameters: Map[String, () => Variant],
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
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[PythonAction] = {
		if (!process.contains("script")) {
			println(s"Python process $name doesn't have a script field")
			None
		}
		if (!process("script").isInstanceOf[StringV]) {
			println(s"Python process $name script isn't a string")
			None
		}
		if (!process.contains("args")) {
			println(s"Python process $name doesn't have a args field")
			None
		}
		if (!process("args").isInstanceOf[StringV]) {
			println(s"Python process $name args isn't a string")
			None
		}
		val script = Utils.toString(process("script"))
		val args   = Utils.toString(process("args"))
		Seq(PythonAction(script, args, pathOp))
	}
}