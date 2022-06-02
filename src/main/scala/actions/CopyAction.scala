package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class CopyAction(filename: String, language: String, srcPath: String)
	extends Action {

	override val phase: Int = 1

	private var dstAbsPath: Path = Path.of("")

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {

		val fn = filename.split('/').last

		val srcAbsPath = Game.projectPath.resolve(Game.resolvePathMacros(instance, srcPath)).toAbsolutePath
		dstAbsPath = Game.outPath.resolve(s"${instance.name}/$fn").toAbsolutePath
		Utils.ensureDirectories(dstAbsPath.getParent)

		val source = Utils.readFile(srcAbsPath)
		Utils.writeFile(dstAbsPath, source.toString)
	}

	def getDestPath: String = dstAbsPath.toString.replace('\\', '/')

}

object CopyAction {
	def apply(name: String, process: Map[String, Variant]): Seq[CopyAction] = {
		if (!process.contains("sources")) {
			println(s"Sources process $name doesn't have a sources field")
			return Seq()
		}

		val srcs = Utils.toArray(process("sources")).map(Utils.toTable)
		for (entry <- srcs) yield {
			val filename = Utils.toString(entry("file"))
			CopyAction(filename, Utils.toString(entry("language")), filename)
		}
	}
}
