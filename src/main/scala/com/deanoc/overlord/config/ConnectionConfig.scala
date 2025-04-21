package com.deanoc.overlord.config

import io.circe.{Decoder, Json, HCursor}
import io.circe.generic.semiauto._

// Base trait for all connection configurations
sealed trait ConnectionConfig {
  def `type`: String
  def connection: String
}

object ConnectionConfig {
  implicit val decoder: Decoder[ConnectionConfig] = (c: HCursor) => {
    for {
      connType <- c.downField("type").as[String]
      result <- connType match {
        case "bus" => c.as[BusConnectionConfig]
        case "port" => c.as[PortConnectionConfig]
        case "port_group" => c.as[PortGroupConnectionConfig]
        case "clock" => c.as[ClockConnectionConfig]
        case "logical" => c.as[LogicalConnectionConfig]
        case "parameters" => c.as[ParametersConnectionConfig]
        case "constant" => c.as[ConstantConnectionConfig]
        case _ => Left(io.circe.DecodingFailure(s"Unknown connection type: $connType", c.history))
      }
    } yield result
  }
}

// Represents a bus connection in the YAML
case class BusConnectionConfig(
  connection: String,
  `type`: String,
  bus_name: String,
  bus_width: Int,
  bus_protocol: String,
  supplier_bus_name: String,
  consumer_bus_name: String,
  silent: Boolean
) extends ConnectionConfig

object BusConnectionConfig {
  import CirceDefaults._

  implicit val decoder: Decoder[BusConnectionConfig] = 
    Decoder.instance { d =>
      for {
        c <- d.downField("connection").as[String]
        t <- d.downField("type").as[String]
        bn <- withDefault(d, "bus_name", "")
        bw <- withDefault(d, "bus_width", 32)
        bp <- withDefault(d, "bus_protocol", "internal")
        sn <- withDefault(d, "supplier_bus_name", bn)
        cn <- withDefault(d, "consumer_bus_name", bn)
        ss <- withDefault(d, "silent", false)
      } yield BusConnectionConfig(c, t, bn, bw, bp, sn, cn, ss)
    }
}

// Represents a port connection in the YAML
case class PortConnectionConfig(
  connection: String,
  `type`: String
) extends ConnectionConfig

object PortConnectionConfig {
  implicit val decoder: Decoder[PortConnectionConfig] = deriveDecoder[PortConnectionConfig]
}

// Represents a port group connection in the YAML
case class PortGroupConnectionConfig(
  connection: String,
  `type`: String,
  first_prefix: Option[String] = None,
  second_prefix: Option[String] = None,
  excludes: Option[List[String]] = None
) extends ConnectionConfig

object PortGroupConnectionConfig {
  implicit val decoder: Decoder[PortGroupConnectionConfig] = deriveDecoder[PortGroupConnectionConfig]
}

// Represents a clock connection in the YAML
case class ClockConnectionConfig(
  connection: String,
  `type`: String
) extends ConnectionConfig

object ClockConnectionConfig {
  implicit val decoder: Decoder[ClockConnectionConfig] = deriveDecoder[ClockConnectionConfig]
}

// Represents a logical connection in the YAML
case class LogicalConnectionConfig(
  connection: String,
  `type`: String
) extends ConnectionConfig

object LogicalConnectionConfig {
  implicit val decoder: Decoder[LogicalConnectionConfig] = deriveDecoder[LogicalConnectionConfig]
}

// Represents a parameters connection in the YAML
case class ParametersConnectionConfig(
  connection: String,
  `type`: String,
  parameters: List[ParameterConfig]
) extends ConnectionConfig

object ParametersConnectionConfig {
  implicit val decoder: Decoder[ParametersConnectionConfig] = deriveDecoder[ParametersConnectionConfig]
}

// Represents a parameter in a parameters connection
case class ParameterConfig(
  name: String,
  value: Json,
  `type`: Option[String] = None
)

object ParameterConfig {
  implicit val decoder: Decoder[ParameterConfig] = deriveDecoder[ParameterConfig]
}

// Represents a constant connection in the YAML
case class ConstantConnectionConfig(
  connection: String,
  `type`: String,
  value: Option[Json] = None
) extends ConnectionConfig

object ConstantConnectionConfig {
  implicit val decoder: Decoder[ConstantConnectionConfig] = deriveDecoder[ConstantConnectionConfig]
}
