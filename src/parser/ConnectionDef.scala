package overlord.parser

import toml._;

trait ConnectionDef {
  val ident : String
  val attributes: Map[String, toml.Value]
}

case class BusDef(
  override val ident : String,
  val busType: String,
  val count: Int,
  override val attributes: Map[String, toml.Value]
) extends ConnectionDef

