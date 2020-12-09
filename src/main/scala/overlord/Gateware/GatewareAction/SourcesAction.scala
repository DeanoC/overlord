package overlord.Gateware.GatewareAction

import java.nio.file.Path
import overlord.Gateware.{Gateware, Parameter}
import overlord.Instances.Instance
import overlord.{Game, Utils}
import toml.Value

case class SourcesAction(filename: String,
                         language: String,
                         srcPath: String,
                         pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(gateware: Instance,
	                     parameters: Map[String, Parameter],
	                     outPath: Path): Unit = {}
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