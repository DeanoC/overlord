package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

import java.nio.file.{Path, Paths}

case class CopyAction(filename: String, language: String, srcPath: String)
	extends Action {

	override val phase: Int = 1

	private var dstAbsPath: Path = Paths.get("")

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {

		val fn = filename.split('/').last

		val srcAbsPath = Project.projectPath.resolve(Project.resolvePathMacros(instance, srcPath)).toAbsolutePath
		dstAbsPath = Project.outPath.resolve(s"${instance.name}/$fn").toAbsolutePath
		Utils.ensureDirectories(dstAbsPath.getParent)

		val source = Utils. readFile(srcAbsPath)
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
		for (entry <- srcs.toIndexedSeq) yield {
			val filename = Utils.toString(entry("file"))
			CopyAction(filename, Utils.toString(entry("language")), filename)
		}
	}
}
