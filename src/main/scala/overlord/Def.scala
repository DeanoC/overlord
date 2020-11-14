package overlord

import toml.Value

trait Definition {
	val chipType  : String
	val container : Option[String]
	val attributes: Map[String, Value]
	val softwares : Seq[Software]
}

case class Def(override val chipType: String,
               override val container: Option[String],
               override val attributes: Map[String, Value],
               override val softwares: Seq[Software] = Seq[Software]())
	extends Definition


