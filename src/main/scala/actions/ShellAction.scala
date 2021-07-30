package actions

import ikuy_utils.Variant

import java.nio.file.Path
import overlord.Instances.ChipInstance

case class ShellAction(script: String,
                       args: String,
                       pathOp: ActionPathOp,
                       phase: Int = 1
                      )
	extends GatewareAction {
	override def execute(instance: ChipInstance, parameters: Map[String, Variant], outPath: Path): Unit = ???
}
