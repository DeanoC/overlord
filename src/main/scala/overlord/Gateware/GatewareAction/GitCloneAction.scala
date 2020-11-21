package overlord.Gateware.GatewareAction

import java.nio.file.Path

import overlord.Gateware.Gateware
import overlord.Instances.Instance
import overlord.Utils
import toml.Value

case class GitCloneAction(url: String,
                          pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(gateware: Instance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = {
		import scala.language.postfixOps
		import scala.sys.process._

		val path = outPath.resolve(url.split('/').last)
		if (!path.toFile.exists()) {
			val result = s"git clone --recursive $url $path" !

			if (result != 0)
				println(s"FAILED git clone of $url")
		}
		updatePath(path)
	}
}

object GitCloneAction {
	def apply(name: String,
	          process: Map[String, Value],
	          pathOp: GatewarePathOp): Seq[GitCloneAction] = {
		if (!process.contains("url")) {
			println(s"Git Clone process $name doesn't have a url field")
			None
		}

		Seq(GitCloneAction(Utils.toString(process("url")), pathOp))
	}
}