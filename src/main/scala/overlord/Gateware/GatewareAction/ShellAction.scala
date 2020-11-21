package overlord.Gateware.GatewareAction

import java.nio.file.Path

import overlord.Gateware.Gateware
import overlord.Instances.Instance

case class ShellAction(script: String,
                       args: String,
                       pathOp: GatewarePathOp)
	extends GatewareAction {
	override def execute(gateware: Instance,
	                     parameters: Map[String, String],
	                     outPath: Path): Unit = ???
}
