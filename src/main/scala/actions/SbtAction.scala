package actions

import java.nio.file.Path
import overlord.Instances.ChipInstance
import overlord.Game
import ikuy_utils._

case class SbtAction(mainScala: String,
                     args: String,
                     withBuildSbt: Boolean,
                     srcPath: String,
                     pathOp: ActionPathOp)
	extends GatewareAction {

	override val phase: Int = 1

	override def execute(instance: ChipInstance,
	                     parameters: Map[String, Variant],
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

		Utils.readFile("build.sbt",
		               srcAbsPath.resolve("build.sbt"), getClass) match {
			case Some(value) =>
				Utils.writeFile(moddedOutPath.resolve("build.sbt"), value)
			case None        =>
				println("built.sbt not found")
				return
		}

		val source = Utils.readFile(mainScala,
		                            srcAbsPath.resolve(mainScala),
		                            getClass)
		if(source.nonEmpty)
			Utils.writeFile(dstAbsPath, source.get)
		else println(s"$mainScala doesn't not exist")

		val proargs = args.replace("${dest}", moddedOutPath.toString)
			.replace("${name}", instance.ident)

		val result =
			sys.process.Process(Seq("sbt", proargs), moddedOutPath.toFile).!

		if (result != 0)
			println(s"FAILED sbt of $args")

		updatePath(moddedOutPath)
	}
}

object SbtAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[SbtAction] = {
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
		Seq(SbtAction(
			mainScala,
			args,
			withBuildSbt,
			Game.pathStack.top.toString,
			pathOp))
	}
}