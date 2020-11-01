package overlord.parser

import toml._;

case class InterconnectDef(
    val ident: String,
    val connecteds: Seq[ConnectionDef],
    val unconnecteds: Seq[String],
    val attributes: Map[String, toml.Value]
)
