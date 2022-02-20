package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class SourcesAction(filename: String,
                         language: String,
                         srcPath: String,
                         pathOp: ActionPathOp)
	extends Action {

	override val phase: Int = 1

	private var actualSrcPath = srcPath

	override def execute(instance: InstanceTrait,
	                     parameters: Map[String, () => Variant],
	                     outPath: Path): Unit = {
		actualSrcPath = srcPath.replace("${name}", instance.ident)
		if (!Path.of(actualSrcPath).toFile.exists()) {
			println(f"$actualSrcPath not found%n")
		}
	}

	def getSrcPath: String = actualSrcPath

}

object SourcesAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[SourcesAction] = {
		if (!process.contains("sources")) {
			println(s"Sources process $name doesn't have a sources field")
			None
		}
		val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

		for (entry <- srcs) yield {
			val filename = Utils.toString(entry("file"))
			val srcPath  = if (filename.contains("${src}")) {
				val tmp = filename.replace("${src}", "")
				Game.pathStack.top.toString + tmp
			} else filename

			SourcesAction(filename,
			              Utils.toString(entry("language")),
			              srcPath,
			              pathOp)
		}
	}
}