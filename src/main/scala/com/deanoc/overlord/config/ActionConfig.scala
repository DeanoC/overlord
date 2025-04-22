package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor, DecodingFailure}
import com.deanoc.overlord.utils.Utils.ReflectionHelper
import scala.annotation.meta.param

sealed trait ActionConfig {
  def attributes: Map[String, Any]
}

object ActionConfig {
  implicit val decoder: Decoder[ActionConfig] = new Decoder[ActionConfig] {
    def apply(c: HCursor): Decoder.Result[ActionConfig] = {
      c.downField("action").as[String].flatMap {
        case "sources" => c.as[SourcesActionConfig]
        case "read_rtl_top" => c.as[ReadRtlTopConfig]
        case _         => Left(DecodingFailure("Unknown action type", c.history))
      }
    }
  }
}

case class SourcesActionConfig(
  language: String,
  files: List[String],
  attributes: Map[String, Any]
) extends ActionConfig
object SourcesActionConfig {  
    implicit val decoder: Decoder[SourcesActionConfig] = (c: HCursor) => {
    for {
      language <- c.downField("language").as[String]
      files <- c.downField("files").as[List[String]]
      attributes = ReflectionHelper.extractUnhandledFields[SourcesActionConfig](c)
    } yield SourcesActionConfig(language, files, attributes)
  }
}
case class ReadRtlTopConfig(
  language: String,
  file: String,
  attributes: Map[String, Any]
) extends ActionConfig

object ReadRtlTopConfig {  
  implicit val decoder: Decoder[ReadRtlTopConfig] = (c: HCursor) => {
  for {
      language <- c.downField("language").as[String]
      file <- c.downField("file").as[String]
      attributes = ReflectionHelper.extractUnhandledFields[ReadRtlTopConfig](c)
    } yield ReadRtlTopConfig(language, file, attributes)
  }
}