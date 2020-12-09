package overlord.Gateware.GatewareAction

import java.nio.file.Path
import overlord.Gateware.{Gateware, Parameter}
import overlord.Instances.Instance

case class ShellAction(script: String,
                       args: String,
                       pathOp: GatewareActionPathOp,
                       phase: GatewareActionPhase = GatewareActionPhase1()
                      )
	extends GatewareAction {
	override def execute(gateware: Instance, parameters: Map[String, Parameter], outPath: Path): Unit = ???
}
