package com.deanoc.overlord.config

import io.circe.{Decoder, Json, HCursor}
import com.deanoc.overlord.config.CustomDecoders._

// Define an enum for source types
enum SourceType {
  case Git, Local, Fetch, Inline

  override def toString: String = this match {
    case Git    => "git"
    case Local  => "local"
    case Fetch  => "fetch"
    case Inline => "inline"
  }
}

object SourceType {
  def fromString(str: String): Option[SourceType] = str.toLowerCase match {
    case "git"    => Some(SourceType.Git)
    case "local"  => Some(SourceType.Local)
    case "fetch"  => Some(SourceType.Fetch)
    case "inline" => Some(SourceType.Inline)
    case _        => None
  }

  implicit val decoder: Decoder[SourceType] = Decoder.decodeString.emap { str =>
    fromString(str).toRight(s"Invalid source type: $str")
  }
}

case class SourceConfig(
  `type`: SourceType,
  url: Option[String] = None,
  path: Option[String] = None,
  inline: Option[Json] = None,
)

object SourceConfig {
  implicit val decoder: Decoder[SourceConfig] = new Decoder[SourceConfig] {
    def apply(c: HCursor): Decoder.Result[SourceConfig] = {
      for {
        t <- c.downField("type").as[SourceType]
        u = t match {
          case SourceType.Git | SourceType.Fetch => c.downField("url").as[String].toOption
          case _                                 => None
        }
        p = t match {
          case SourceType.Local => c.downField("path").as[String].toOption
          case _                => None
        }
        i = t match {
          case SourceType.Inline => c.downField("inline").as[Json].toOption
          case _                 => None
        }
      } yield SourceConfig(t, u, p, i)
    }
  }
}

// Common trait for catalog/component file configuration
trait FileConfigBase {
  def defaults: Map[String, Any]
  def catalogs: List[SourceConfig]
  def definitions: List[DefinitionConfig]
}

// Represents the top-level structure of a catalog YAML file
case class CatalogFileConfig(
  defaults: Map[String, Any] = Map.empty,
  catalogs: List[SourceConfig] = List.empty,
  definitions: List[DefinitionConfig] = List.empty
) extends FileConfigBase

object CatalogFileConfig {
  implicit val decoder: Decoder[CatalogFileConfig] = new Decoder[CatalogFileConfig] {
    def apply(c: HCursor): Decoder.Result[CatalogFileConfig] = {
      for {
        defaults <- c.downField("defaults").as[Option[Map[String, Any]]].map(_.getOrElse(Map.empty))
        catalogs <- c.downField("catalogs").as[Option[List[SourceConfig]]].map(_.getOrElse(List.empty))
        definitions <- c.downField("definitions").as[Option[List[DefinitionConfig]]].map(_.getOrElse(List.empty))
      } yield CatalogFileConfig(defaults, catalogs, definitions)
    }
  }
}

// Represents the top-level structure of the project YAML file
case class ComponentFileConfig(
  info: InfoConfig,
  components: List[SourceConfig] = List.empty,
  instances: List[InstanceConfig] = List.empty,
  connections: List[ConnectionConfig] = List.empty,
  defaults: Map[String, Any] = Map.empty,
  catalogs: List[SourceConfig] = List.empty,
  definitions: List[DefinitionConfig] = List.empty
) extends FileConfigBase

object ComponentFileConfig {
  implicit val decoder: Decoder[ComponentFileConfig] = new Decoder[ComponentFileConfig] {
    def apply(c: HCursor): Decoder.Result[ComponentFileConfig] = {
      for {
        info <- c.downField("info").as[InfoConfig]
        components <- c.downField("components").as[Option[List[SourceConfig]]].map(_.getOrElse(List.empty))
        instances <- c.downField("instances").as[Option[List[InstanceConfig]]].map(_.getOrElse(List.empty))
        connections <- c.downField("connections").as[Option[List[ConnectionConfig]]].map(_.getOrElse(List.empty))
        defaults <- c.downField("defaults").as[Option[Map[String, Any]]].map(_.getOrElse(Map.empty))
        catalogs <- c.downField("catalogs").as[Option[List[SourceConfig]]].map(_.getOrElse(List.empty))
        definitions <- c.downField("definitions").as[Option[List[DefinitionConfig]]].map(_.getOrElse(List.empty))
      } yield ComponentFileConfig(
        info, components, instances, connections, defaults, catalogs, definitions
      )
    }
  }
}

