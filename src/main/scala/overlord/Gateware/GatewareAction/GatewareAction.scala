package overlord.Gateware.GatewareAction

import java.nio.file.Path

import overlord.GameBuilder
import overlord.Gateware.Gateware
import overlord.Instances.Instance

sealed trait GatewarePathOp

case class GatewarePathOp_Noop() extends GatewarePathOp

case class GatewarePathOp_Push() extends GatewarePathOp

case class GatewarePathOp_Pop() extends GatewarePathOp

trait GatewareAction {
	def execute(gateware: Instance,
	            parameters: Map[String, String],
	            outPath: Path): Unit

	val pathOp: GatewarePathOp

	def updatePath(path: Path): Unit = {
		pathOp match {
			case GatewarePathOp_Noop() =>
			case GatewarePathOp_Push() => GameBuilder.pathStack.push(path)
			case GatewarePathOp_Pop()  => GameBuilder.pathStack.pop()
		}
	}
}
