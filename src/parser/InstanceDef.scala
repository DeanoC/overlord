package overlord.parser

import toml._;

trait InstanceBaseDef {
  val ident: String
  val chip: ChipDef
  val attributes: Map[String, toml.Value]
}

case class InstanceDef(
  override val ident: String,
  override val chip: ChipDef,
  override val attributes: Map[String, toml.Value]
) extends InstanceBaseDef

case class RamInstanceDef(
  override val ident : String,
  val chip: ChipDef,
  override val attributes: Map[String, toml.Value]
) extends InstanceBaseDef with ConnectionDef
