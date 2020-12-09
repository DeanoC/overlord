package overlord.Definitions

import overlord.Gateware.GatewareAction.GatewareAction
import overlord.Gateware.{Parameter, Port}

import scala.collection.mutable

trait GatewareTrait {
	val actions   : Seq[GatewareAction]
	val moduleName: String
	val ports     : mutable.HashMap[String, Port]
	val parameters: mutable.HashMap[String, Parameter]
	val verilog_parameters: mutable.HashSet[String]
}