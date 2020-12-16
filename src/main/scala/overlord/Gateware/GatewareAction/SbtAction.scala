package overlord.Gateware.GatewareAction

import java.nio.file.Path
import overlord.Gateware.{Gateware, Parameter}
import overlord.Instances.Instance
import overlord.Game
import ikuy_utils._
import toml.Value

case class SbtAction(
	                    mainScala: String,
	                    args: String,
	                    withBuildSbt: Boolean,
	                    srcPath: String,
	                    pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(instance: Instance,
	                     parameters: Map[String, Parameter],
	                     outPath: Path): Unit = {
		import scala.language.postfixOps

		val srcAbsPath = if (srcPath.contains("${dest}/")) {
			Game.pathStack.top.resolve(
				srcPath.replace("${dest}/", ""))
		} else Path.of(srcPath)

		val moddedOutPath = if (!withBuildSbt) outPath
		else Game.pathStack.top.resolve(instance.ident)

		val dstAbsPath = moddedOutPath.resolve(mainScala)
		Utils.ensureDirectories(moddedOutPath)

		if (withBuildSbt) {
			val source = Utils.readFile(srcAbsPath.resolve("build.sbt"))
			Utils.writeFile(moddedOutPath.resolve("build.sbt"), source)
		}

		val source = Utils.readFile(srcAbsPath.resolve(mainScala))
		Utils.writeFile(dstAbsPath, source)

		val proargs = args.replace("${dest}", moddedOutPath.toString)
			.replace("${name}", instance.ident)

		val result  =
			sys.process.Process(Seq("sbt", proargs), moddedOutPath.toFile).!

		if (result != 0)
			println(s"FAILED sbt of $args")

		updatePath(moddedOutPath)
	}
}

object SbtAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewareActionPathOp): Seq[SbtAction] = {
		if (!process.contains("args")) {
			println(s"SBT process $name doesn't have a args field")
			None
		}
		if (!process.contains("main_scala")) {
			println(s"SBT process $name doesn't have a main_scala field")
			None
		}

		if (!process("args").isInstanceOf[Value.Str]) {
			println(s"SBT process $name args isn't a string")
			None
		}
		val withBuildSbt =
			if (process.contains("with_build_sbt"))
				Utils.toBoolean(process("with_build_sbt"))
			else false

		val mainScala = Utils.toString(process("main_scala"))
		val args      = Utils.toString(process("args"))
		Seq(SbtAction(
			mainScala,
			args,
			withBuildSbt,
			Game.pathStack.top.toString,
			pathOp))
	}
}