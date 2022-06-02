package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

import scala.collection.mutable

case class TomlAction(filename: String)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		val sb = new mutable.StringBuilder()
		for {(k, vf) <- parameters} {
			sb ++= s"$k] = ${vf.toTomlString}\n"
		}

		val moddedOutPath = Game.outPath.resolve(Game.resolvePathMacros(instance, instance.name))
		val dstAbsPath    = moddedOutPath.resolve(Game.resolvePathMacros(instance, filename))
		Utils.ensureDirectories(dstAbsPath.getParent)

		Utils.writeFile(dstAbsPath, sb.result())
	}

}

object TomlAction {
	def apply(name: String, process: Map[String, Variant]): Seq[TomlAction] = {
		if (!process.contains("parameters")) {
			println(s"Toml process $name doesn't have a parameters field")
			return Seq()
		}
		if (!process("parameters").isInstanceOf[ArrayV]) {
			println(s"Toml process $name parameters isn't an array")
			return Seq()
		}
		if (!process.contains("filename")) {
			println(s"Toml process $name doesn't have a filename field")
			return Seq()
		}
		if (!process("filename").isInstanceOf[StringV]) {
			println(s"Toml process $name filename isn't a string")
			return Seq()
		}

		Seq(TomlAction(Utils.toString(process("filename"))))
	}
}