package actions

import java.nio.file.Path
import overlord.Instances.ChipInstance
import overlord.Game
import ikuy_utils._
import toml.Value

case class YamlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: ActionPathOp)
	extends GatewareAction {

	override val phase: Int = 1

	override def execute(instance: ChipInstance,
	                     parameters: Map[String, Variant],
	                     outPath: Path): Unit = {
		val sb = new StringBuilder()
		for {k <- parameterKeys
		     if parameters.contains(k)}
			sb ++= (parameters(k) match {
				case ArrayV(arr)       => "TODO"
				case BigIntV(bigInt)   => s"$k: $bigInt\n"
				case BooleanV(boolean) => s"$k: $boolean\n"
				case IntV(int)         => s"$k: $int\n"
				case TableV(table)     => "TODO:"
				case StringV(string)   => s"$k: '$string'\n"
				case DoubleV(dbl)      => s"$k: $dbl\n"
			})

		val moddedOutPath = Game.pathStack.top.resolve(instance.ident)

		val dstAbsPath = moddedOutPath.resolve(filename)
		Utils.ensureDirectories(dstAbsPath.getParent)

		Utils.writeFile(dstAbsPath, sb.result())

		updatePath(dstAbsPath.getParent)
	}
}

object YamlAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[YamlAction] = {
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