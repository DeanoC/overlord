package actions

import ikuy_utils.Variant
import overlord.Instances.InstanceTrait

case class ShellAction(script: String,
                       args: String,
                       phase: Int = 1
                      )
	extends Action {
	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = ???
}
