package overlord

import toml.Value

case class GatewareSourceFile(filename : String,
                              language : String,
                              data : String)

trait GatewareTrait {
  val provision: String
  val sources: Seq[GatewareSourceFile]
  val ports: Seq[String]
  val parameters: Seq[String]
}

trait Definition {
  val chipType: String
  val container: Option[String]
  val attributes: Map[String, Value]
  val softwares: Seq[Software]
}
case class Def(override val chipType: String,
               override val container: Option[String],
               override val attributes: Map[String, Value],
               override val softwares:Seq[Software] = Seq[Software]())
extends Definition


