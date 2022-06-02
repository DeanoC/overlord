package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

case class SbtAction(mainScala: String, args: String, srcPath: String)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		import scala.language.postfixOps

		val srcAbsPath    = Game.tryPaths(instance, srcPath)
		val moddedOutPath = Game.outPath.resolve(instance.name)

		Utils.ensureDirectories(moddedOutPath)

		srcAbsPath.toFile.listFiles.foreach(f => if (f.isFile) Utils.copy(f.toPath, moddedOutPath.resolve(f.toPath.getFileName)))

		val proargs = Game.resolvePathMacros(instance, args)

		val result = sys.process.Process(Seq("sbt", proargs), moddedOutPath.toFile).!

		if (result != 0) println(s"FAILED sbt of ${instance.name} with $args")
	}
}

object SbtAction {
	def apply(name: String, process: Map[String, Variant]): Seq[SbtAction] = {
		if (!process.contains("args")) {
			println(s"SBT process $name doesn't have a args field")
			None
		}
		if (!process.contains("main_scala")) {
			println(s"SBT process $name doesn't have a main_scala field")
			None
		}

		if (!process("args").isInstanceOf[StringV]) {
			println(s"SBT process $name args isn't a string")
			None
		}
		val withBuildSbt =
			if (process.contains("with_build_sbt"))
				Utils.toBoolean(process("with_build_sbt"))
			else false

		val mainScala = Utils.toString(process("main_scala"))
		val args      = Utils.toString(process("args"))

		Seq(SbtAction(mainScala, args, Game.catalogPath.toString))
	}
}