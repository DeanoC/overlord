package overlord.Gateware.GatewareAction

import overlord.Gateware.Parameter

import java.nio.file.Path
import overlord.Instances.Instance
import overlord.Game
import ikuy_utils._
import toml.Value

import scala.util.{Failure, Success, Try}

case class YamlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(instance: Instance, parameters: Map[String, Parameter], outPath: Path): Unit = {
		val sb = new StringBuilder()
		for {k <- parameterKeys
		     if parameters.contains(k)} sb ++= {
			val v = parameters(k).value
			Try {
				v.toLong
			} match {
				case Failure(_)     => s"$k: '$v'\n"
				case Success(value) => s"$k: $value\n"
			}
		}

		val moddedOutPath = Game.pathStack.top.resolve(instance.ident)

		val dstAbsPath = moddedOutPath.resolve(filename)
		Utils.ensureDirectories(dstAbsPath.getParent)

		Utils.writeFile(dstAbsPath, sb.result())

		updatePath(dstAbsPath.getParent)
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