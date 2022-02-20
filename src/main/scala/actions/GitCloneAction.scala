package actions

import ikuy_utils._
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class GitCloneAction(url: String,
                          pathOp: ActionPathOp)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait,
	                     parameters: Map[String, () => Variant],
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
	          process: Map[String, Variant],
	          pathOp: ActionPathOp): Seq[GitCloneAction] = {
		if (!process.contains("url")) {
			println(s"Git Clone process $name doesn't have a url field")
			None
		}

		Seq(GitCloneAction(Utils.toString(process("url")), pathOp))
	}
}