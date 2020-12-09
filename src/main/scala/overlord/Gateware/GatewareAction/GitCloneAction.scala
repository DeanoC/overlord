package overlord.Gateware.GatewareAction

import java.nio.file.Path
import overlord.Gateware.{Gateware, Parameter}
import overlord.Instances.Instance
import overlord.Utils
import toml.Value

case class GitCloneAction(url: String,
                          pathOp: GatewareActionPathOp)
	extends GatewareAction {

	override val phase: GatewareActionPhase = GatewareActionPhase1()

	override def execute(gateware: Instance, parameters: Map[String, Parameter], outPath: Path): Unit = {
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
	          pathOp: GatewareActionPathOp): Seq[GitCloneAction] = {
		if (!process.contains("url")) {
			println(s"Git Clone process $name doesn't have a url field")
			None
		}

		Seq(GitCloneAction(Utils.toString(process("url")), pathOp))
	}
}