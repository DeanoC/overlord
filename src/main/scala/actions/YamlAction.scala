package actions

import gagameos._
import overlord.Game
import overlord.Instances.InstanceTrait

import scala.collection.mutable

case class YamlAction(parameterKeys: Seq[String], filename: String)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		val sb = new mutable.StringBuilder()
		for {(k, v) <- parameters}
			sb ++= (v match {
				case ArrayV(arr)       => ???
				case BigIntV(bigInt)   => s"$k: $bigInt\n"
				case BooleanV(boolean) => s"$k: $boolean\n"
				case IntV(int)         => s"$k: $int\n"
				case TableV(table)     => ???
				case StringV(string)   => s"$k: '$string'\n"
				case DoubleV(dbl)      => s"$k: $dbl\n"
			})

		val moddedOutPath = Game.outPath.resolve(Game.resolvePathMacros(instance, instance.name))

		val dstAbsPath = moddedOutPath.resolve(Game.resolvePathMacros(instance, filename))
		Utils.ensureDirectories(dstAbsPath.getParent)

		Utils.writeFile(dstAbsPath, sb.result())
	}
}

object YamlAction {
	def apply(name: String, process: Map[String, Variant]): Seq[YamlAction] = {
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

		Seq(YamlAction(parameters.toIndexedSeq, filename))
	}
}