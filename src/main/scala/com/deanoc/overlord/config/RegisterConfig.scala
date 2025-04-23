package com.deanoc.overlord.config

import io.circe.Decoder

case class FieldConfig(
  bits: String, // Bit range "high:low"
  name: String, // Field identifier
  `type`: String, // Access type: raz/rw/ro/wo/mixed
  shortdesc: Option[String] = None, // Brief functional description
  longdesc: Option[String] = None // Detailed technical documentation
) derives Decoder

case class RegisterConfig(
  default: String, // Power-on value (e.g., "0x00000000")
  description: String, // Functional purpose
  field: List[FieldConfig], // Bitfield definitions
  name: String, // Register name (e.g., "MCU_RESET")
  offset: String, // Address offset from bank base
  `type`: String, // Access type: mixed/rw/ro/wo
  width: String // Bit width (e.g., "32")
) derives Decoder
