package com.deanoc.overlord.config

import io.circe.{Decoder, HCursor, DecodingFailure}

sealed trait ActionConfig

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
  files: List[String]
) extends ActionConfig derives Decoder

case class ReadRtlTopConfig(
  language: String,
  file: String
) extends ActionConfig derives Decoder