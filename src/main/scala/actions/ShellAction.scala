package actions

import gagameos.Variant
import overlord.Instances.InstanceTrait

case class ShellAction(script: String,
                       args: String,
                       phase: Int = 1
                      )
	extends Action {
	override def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit = ???
}
