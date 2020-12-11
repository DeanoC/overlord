package overlord.Gateware.GatewareAction

import overlord.Gateware.Parameter
import overlord.Instances.Instance
import overlord.{Game, Utils}
import toml.Value

import java.nio.file.Path

case class TomlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(instance: Instance, parameters: Map[String, Parameter], outPath: Path): Unit = {
		val sb = new StringBuilder()
		for {k <- parameterKeys
		     if parameters.contains(k)} sb ++= {
			parameters(k).value match {
				case v: Value.Str  => s"$k = '${v.value}'\n"
				case v: Value.Bool => s"$k = ${v.value}\n"
				case v: Value.Num  => s"$k = ${v.value}\n"
				case v: Value.Real => s"$k = ${v.value}\n"

				case _: Value.Arr  => assert(false); ""
				case _: Value.Tbl  => assert(false); ""
			}
		}

		val moddedOutPath = Game.pathStack.top.resolve(instance.ident)

		val dstAbsPath = moddedOutPath.resolve(filename)
		Utils.ensureDirectories(dstAbsPath.getParent)

		Utils.writeFile(dstAbsPath, sb.result())

		updatePath(dstAbsPath.getParent)
	}
}

object TomlAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewareActionPathOp): Seq[TomlAction] = {
		if (!process.contains("parameters")) {
			println(s"Toml process $name doesn't have a parameters field")
			None
		}
		if (!process("parameters").isInstanceOf[Value.Arr]) {
			println(s"Toml process $name parameters isn't an array")
			None
		}
		if (!process.contains("filename")) {
			println(s"Toml process $name doesn't have a filename field")
			None
		}
		if (!process("filename").isInstanceOf[Value.Str]) {
			println(s"Toml process $name filename isn't a string")
			None
		}

		val filename   = Utils.toString(process("filename"))
		val parameters = Utils.toArray(process("parameters")).map(Utils.toString)

		Seq(TomlAction(parameters, filename, pathOp))
	}
}