package overlord.Definitions

import ikuy_utils.Variant
import overlord.Gateware.GatewareAction.GatewareAction
import overlord.Gateware.Port

import scala.collection.mutable

trait GatewareTrait {
	val actions   : Seq[GatewareAction]
	val ports     : mutable.HashMap[String, Port]
	val parameters: Map[String, Variant]
}