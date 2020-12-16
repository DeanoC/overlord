package overlord.Gateware.GatewareAction

import java.nio.file.Path
import overlord.Gateware.{Gateware, Parameter}
import overlord.Instances.Instance
import overlord.Game
import ikuy_utils._
import toml.Value

case class SourcesAction(filename: String,
                         language: String,
                         srcPath: String,
                         pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	private var actualSrcPath = srcPath

	override def execute(instance: Instance,
	                     parameters: Map[String, Parameter],
	                     outPath: Path): Unit = {
		actualSrcPath = srcPath.replace("${name}", instance.ident)

	}

	def getSrcPath: String = actualSrcPath

}

object SourcesAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewareActionPathOp): Seq[SourcesAction] = {
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