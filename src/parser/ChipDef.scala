package overlord.parser

import toml._;

case class ChipDef(
  val chipType: String,
  val attributes: Map[String, toml.Value]
)