package overlord.Definitions

import overlord.Gateware.GatewareAction.GatewareAction

trait GatewareTrait {
	val actions   : Seq[GatewareAction]
	val moduleName: String
	val ports     : Seq[String]
	val parameters: Map[String, String]
}


