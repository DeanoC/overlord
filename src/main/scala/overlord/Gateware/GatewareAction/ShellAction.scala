package overlord.Gateware.GatewareAction

import ikuy_utils.Variant

import java.nio.file.Path
import overlord.Instances.Instance

case class ShellAction(script: String,
                       args: String,
                       pathOp: GatewareActionPathOp,
                       phase: GatewareActionPhase = GatewareActionPhase1()
                      )
	extends GatewareAction {
	override def execute(gateware: Instance, parameters: Map[String, Variant], outPath: Path): Unit = ???
}
