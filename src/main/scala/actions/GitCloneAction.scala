package actions

import ikuy_utils._
import overlord.Game
import overlord.Instances.InstanceTrait

case class GitCloneAction(url: String)
	extends Action {

	override val phase: Int = 1

	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = {
		import scala.language.postfixOps
		import scala.sys.process._

		val path = Game.outPath.resolve(url.split('/').last)
		if (!path.toFile.exists()) {
			val result = s"git clone --recursive $url $path" !

			if (result != 0)
				println(s"FAILED git clone of $url")
		}
	}
}

object GitCloneAction {
	def apply(name: String,
	          process: Map[String, Variant]): Seq[GitCloneAction] = {
		if (!process.contains("url")) {
			println(s"Git Clone process $name doesn't have a url field")
			return Seq()
		}

		???

		Seq(GitCloneAction(Utils.toString(process("url"))))
	}
}