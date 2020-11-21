package overlord.Gateware.GatewareAction

import java.nio.file.Path

import overlord.Gateware.Gateware
import overlord.Instances.Instance
import overlord.{GameBuilder, Utils}
import toml.Value

case class SourcesAction(filename: String,
                         language: String,
                         srcPath: String,
                         pathOp: GatewarePathOp)
	extends GatewareAction {

	override def execute(gateware: Instance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {}
}

object SourcesAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewarePathOp): Seq[SourcesAction] = {
		if (!process.contains("sources")) {
			println(s"Sources process $name doesn't have a sources field")
			None
		}
		val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

		for (entry <- srcs) yield {
			val filename = Utils.toString(entry("file"))
			val srcPath  = if (filename.contains("${src}")) {
				val tmp = filename.replace("${src}", "")
				GameBuilder.pathStack.top.toString + tmp
			} else filename

			SourcesAction(filename,
			              Utils.toString(entry("language")),
			              srcPath,
			              pathOp)
		}
	}
}