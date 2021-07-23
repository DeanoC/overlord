package overlord.Gateware.GatewareAction

import ikuy_utils.Variant

import java.nio.file.Path
import overlord.Game
import overlord.Instances.Instance

sealed trait GatewareActionPathOp

case class GatewareActionPathOp_Noop() extends GatewareActionPathOp

case class GatewareActionPathOp_Push() extends GatewareActionPathOp

case class GatewareActionPathOp_Pop() extends GatewareActionPathOp

sealed trait GatewareActionPhase

case class GatewareActionPhase1() extends GatewareActionPhase

case class GatewareActionPhase2() extends GatewareActionPhase

trait GatewareAction {
	def execute(gateware: Instance,
	            parameters: Map[String, Variant],
	            outPath: Path): Unit

	val phase : GatewareActionPhase
	val pathOp: GatewareActionPathOp

	def isPhase1: Boolean = phase.isInstanceOf[GatewareActionPhase1]

	def isPhase2: Boolean = phase.isInstanceOf[GatewareActionPhase2]

	def updatePath(path: Path): Unit = {
		pathOp match {
			case GatewareActionPathOp_Noop() =>
			case GatewareActionPathOp_Push() => Game.pathStack.push(path)
			case GatewareActionPathOp_Pop()  => Game.pathStack.pop()
		}
	}
}
