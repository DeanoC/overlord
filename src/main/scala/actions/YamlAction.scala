package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class YamlAction(parameterKeys: Seq[String],
                      filename: String,
                      pathOp: ActionPathOp)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait,
	                     parameters: Map[String, () => Variant],
	                     outPath: Path): Unit = {
		val sb = new StringBuilder()
		for {(k, v) <- parameters
		     parameter = v()}
			sb ++= (parameter match {
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
			return Seq()
		}
		if (!process("parameters").isInstanceOf[ArrayV]) {
			println(s"Yaml process $name parameters isn't an array")
			return Seq()
		}
		if (!process.contains("filename")) {
			println(s"Yaml process $name doesn't have a filename field")
			return Seq()
		}
		if (!process("filename").isInstanceOf[StringV]) {
			println(s"Yaml process $name filename isn't a string")
			return Seq()
		}

		val filename   = Utils.toString(process("filename"))
		val parameters = Utils.toArray(process("parameters")).map(Utils.toString)

		Seq(YamlAction(parameters, filename, pathOp))
	}
}