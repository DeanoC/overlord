package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

case class SourcesAction(filename: String, language: String)
	extends Action {

	override val phase: Int = 1

	var actualSrcPath: String = ""

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		val srcNamePath = Game.tryPaths(instance, filename)
		val srcAbsPath  = srcNamePath.toAbsolutePath

		if (!srcAbsPath.toFile.exists()) {
			println(f"SourceAction: $srcAbsPath not found%n")
		}

		actualSrcPath = srcAbsPath.toString // TODO clean this
	}

	def getSrcPath: String = actualSrcPath

}

object SourcesAction {
	def apply(name: String, process: Map[String, Variant]): Seq[SourcesAction] = {
		if (!process.contains("sources")) {
			println(s"Sources process $name doesn't have a sources field")
			return Seq()
		}

		val srcs = Utils.toArray(process("sources")).map(Utils.toTable)
		for (entry <- srcs) yield {
			val filename = Utils.toString(entry("file"))
			SourcesAction(filename, Utils.toString(entry("language")))
		}
	}
}