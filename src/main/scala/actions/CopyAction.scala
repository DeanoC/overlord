package actions

import java.nio.file.Path
import overlord.Instances.ChipInstance
import overlord.Game
import ikuy_utils._

case class CopyAction(filename: String,
                      language: String,
                      srcPath: String,
                      pathOp: ActionPathOp)
	extends GatewareAction {

	override val phase: Int = 1

	private var dstAbsPath: Path = Path.of("")

	override def execute(instance: ChipInstance, parameters: Map[String, Variant], outPath: Path): Unit = {
		val fn         = filename.split('/').last
		val pathString = s"${instance.ident}/$fn"
		dstAbsPath = outPath.resolve(pathString.replace("${dest}/", ""))

		val srcAbsPath = if (srcPath.contains("${dest}/")) {
			Game.pathStack.top.resolve(
				srcPath.replace("${dest}/", ""))
		} else Path.of(srcPath)

		Utils.ensureDirectories(dstAbsPath.getParent)
		val source = Utils.readFile(fn, srcAbsPath, getClass)
		Utils.writeFile(dstAbsPath, source.toString)

		updatePath(dstAbsPath.getParent)

	}

	def getDestPath: String =
		dstAbsPath.toString.replace('\\', '/')

}

object CopyAction {
	def apply(name: String,
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[CopyAction] = {
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

			CopyAction(filename, Utils.toString(entry("language")), srcPath, pathOp)
		}
	}
}
