package actions

import ikuy_utils.Variant
import overlord.Instances.InstanceTrait

import java.nio.file.Path

case class ShellAction(script: String,
                       args: String,
                       pathOp: ActionPathOp,
                       phase: Int = 1
                      )
	extends Action {
	override def execute(instance: InstanceTrait,
	                     parameters: Map[String, () => Variant],
	                     outPath: Path): Unit = ???
}
