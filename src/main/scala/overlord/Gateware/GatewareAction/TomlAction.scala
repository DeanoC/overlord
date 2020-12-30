package overlord.Gateware.GatewareAction

import overlord.Instances.Instance
import overlord.Game
import ikuy_utils._

import java.nio.file.Path

case class TomlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(instance: Instance,
	                     parameters: Map[String, Variant],
	                     outPath: Path): Unit = {
		val sb = new StringBuilder()
		for {k <- parameterKeys
		     if parameters.contains(k)} {
			sb ++= s"$k = ${parameters(k).toString}\n"
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
	          process: Map[String, Variant],
	          pathOp: GatewareActionPathOp): Seq[TomlAction] = {
		if (!process.contains("parameters")) {
			println(s"Toml process $name doesn't have a parameters field")
			None
		}
		if (!process("parameters").isInstanceOf[ArrayV]) {
			println(s"Toml process $name parameters isn't an array")
			None
		}
		if (!process.contains("filename")) {
			println(s"Toml process $name doesn't have a filename field")
			None
		}
		if (!process("filename").isInstanceOf[StringV]) {
			println(s"Toml process $name filename isn't a string")
			None
		}

		val filename   = Utils.toString(process("filename"))
		val parameters = Utils.toArray(process("parameters")).map(Utils.toString)

		Seq(TomlAction(parameters, filename, pathOp))
	}
}