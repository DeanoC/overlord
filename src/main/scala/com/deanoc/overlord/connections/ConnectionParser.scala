package com.deanoc.overlord.connections

import com.deanoc.overlord._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.utils.{Utils, Variant, Logging, StringV}
import com.deanoc.overlord.config.BitsDesc
import com.deanoc.overlord.hardware.HardwareBoundrary
import com.deanoc.overlord.interfaces._
import com.deanoc.overlord.instances.{
  HardwareInstance,
  InstanceTrait,
  PinGroupInstance
}
import com.deanoc.overlord.connections.ConnectionTypes.{BusName}
import com.deanoc.overlord.config._
import com.deanoc.overlord.config.WireDirection
import io.circe.{Json, JsonObject}

/** Module containing parsers for various connection types.
  */
object ConnectionParser extends Logging {

  /** Parse a string into a ConnectionPriority
    *
    * @param str
    *   The string representation of the priority
    * @return
    *   An Option containing the ConnectionPriority or None if invalid
    */
  def parseConnectionPriority(str: String): Option[ConnectionPriority] =
    str.toLowerCase match
      case "explicit" => Some(ConnectionPriority.Explicit)
      case "group"    => Some(ConnectionPriority.Group)
      case "wildcard" => Some(ConnectionPriority.WildCard)
      case "fake"     => Some(ConnectionPriority.Fake)
      case _          => None

  /** Parses a connection configuration and creates the appropriate
    * UnconnectedLike instance.
    *
    * @param config
    *   The connection configuration to parse.
    * @return
    *   An optional UnconnectedLike instance based on the parsed connection.
    */
  def parseConnectionConfig(
      config: ConnectionConfig
  ): Option[UnconnectedLike] = {
    val cons = config.connection

    // More robust parsing of connection string that handles extra whitespace
    // First, identify which connection operator is present
    val connectionPattern = "(.+?)\\s*(<->|<>|->|<-)\\s*(.+)".r

    val (first, dirSymbol, secondary) = cons match {
      case connectionPattern(left, op, right) =>
        // Check that the right side doesn't contain any more operators
        if (
          right.contains("->") || right
            .contains("<-") || right.contains("<>") || right.contains("<->")
        ) {
          error(
            s"${config.`type`} has an invalid connection field with multiple operators: $cons"
          )
          return None
        }
        (left.trim, op, right.trim)
      case _ =>
        error(s"${config.`type`} has an invalid connection field: $cons")
        return None
    }

    // Parse the connection direction from the identified symbol
    val dir = dirSymbol match {
      case "->"         => ConnectionDirection.FirstToSecond
      case "<->" | "<>" => ConnectionDirection.BiDirectional
      case "<-"         => ConnectionDirection.SecondToFirst
      case _ =>
        error(s"${config.`type`} has an invalid connection $dirSymbol : $cons")
        return None
    }

    // Create the appropriate connection type based on the configuration type
    // Use dependency injection by passing the specific configuration object
    config match {
      case portConfig: PortConnectionConfig =>
        // Inject the PortConnectionConfig
        Some(createUnconnectedPortGroup(first, dir, secondary, portConfig))

      case clockConfig: ClockConnectionConfig =>
        // Inject the ClockConnectionConfig
        Some(createUnconnectedClock(first, dir, secondary, clockConfig))

      case paramsConfig: ParametersConnectionConfig =>
        if (first != "_") None
        else
          // Inject the ParametersConnectionConfig
          Some(
            parseParametersConnectionConfig(
              dir,
              secondary,
              paramsConfig.parameters
            )
          )

      case portGroupConfig: PortGroupConnectionConfig =>
        // Inject the PortGroupConnectionConfig
        Some(createUnconnectedPortGroup(first, dir, secondary, portGroupConfig))

      case busConfig: BusConnectionConfig =>
        // Inject the BusConnectionConfig
        Some(parseBusConnectionConfig(first, dir, secondary, busConfig))

      case logicalConfig: LogicalConnectionConfig =>
        // Inject the LogicalConnectionConfig
        Some(createUnconnectedLogical(first, dir, secondary, logicalConfig))

      case constantConfig: ConstantConnectionConfig =>
        // Handle constant connections if needed
        error(s"Constant connections not yet implemented")
        None

      case null =>
        error(s"${config.`type`} is an unknown connection type")
        None
    }
  }

  // Helper methods to create unconnected instances with injected configuration

  private def createUnconnectedPortGroup(
      first: String,
      dir: ConnectionDirection,
      secondary: String,
      config: PortConnectionConfig
  ): UnconnectedPortGroup = {
    UnconnectedPortGroup(first, dir, secondary)
  }

  private def createUnconnectedPortGroup(
      first: String,
      dir: ConnectionDirection,
      secondary: String,
      config: PortGroupConnectionConfig
  ): UnconnectedPortGroup = {
    UnconnectedPortGroup(
      first,
      dir,
      secondary,
      config.first_prefix.getOrElse(""),
      config.second_prefix.getOrElse(""),
      config.excludes.map(_.toSeq).getOrElse(Seq.empty)
    )
  }

  private def createUnconnectedClock(
      first: String,
      dir: ConnectionDirection,
      secondary: String,
      config: ClockConnectionConfig
  ): UnconnectedClock = {
    UnconnectedClock(first, dir, secondary)
  }

  private def createUnconnectedLogical(
      first: String,
      dir: ConnectionDirection,
      secondary: String,
      config: LogicalConnectionConfig
  ): UnconnectedLogical = {
    UnconnectedLogical(first, dir, secondary)
  }

  /** Parses a connection variant and creates the appropriate UnconnectedLike
    * instance.
    *
    * @param connection
    *   The connection variant to parse.
    * @return
    *   An optional UnconnectedLike instance based on the parsed connection.
    */
  def parseConnection(connection: Variant): Option[UnconnectedLike] = {
    val table = Utils.toTable(connection)

    if (!table.contains("type")) {
      error(s"connection $connection requires a type field")
      return None
    }
    val conntype = Utils.toString(table("type"))

    if (!table.contains("connection")) {
      error(s"connection $conntype requires a connection field")
      return None
    }

    val cons = Utils.toString(table("connection"))

    // More robust parsing of connection string that handles extra whitespace
    // First, identify which connection operator is present
    val connectionPattern = "(.+?)\\s*(<->|<>|->|<-)\\s*(.+)".r

    val (first, dirSymbol, secondary) = cons match {
      case connectionPattern(left, op, right) =>
        // Check that the right side doesn't contain any more operators
        if (
          right.contains("->") || right
            .contains("<-") || right.contains("<>") || right.contains("<->")
        ) {
          error(
            s"$conntype has an invalid connection field with multiple operators: $cons"
          )
          return None
        }
        (left.trim, op, right.trim)
      case _ =>
        error(s"$conntype has an invalid connection field: $cons")
        return None
    }

    // Parse the connection direction from the identified symbol
    val dir = dirSymbol match {
      case "->"         => ConnectionDirection.FirstToSecond
      case "<->" | "<>" => ConnectionDirection.BiDirectional
      case "<-"         => ConnectionDirection.SecondToFirst
      case _ =>
        error(s"$conntype has an invalid connection $dirSymbol : $cons")
        return None
    }

    // Create the appropriate connection type based on the "type" field
    conntype match {
      case "port" =>
        Some(UnconnectedPortGroup(first, dir, secondary))

      case "clock" =>
        Some(UnconnectedClock(first, dir, secondary))

      case "parameters" =>
        if (first != "_") None
        else
          Some(
            parseParametersConnection(
              dir,
              secondary,
              Utils.lookupArray(table, "parameters")
            )
          )

      case "port_group" =>
        Some(
          UnconnectedPortGroup(
            first,
            dir,
            secondary,
            Utils.lookupString(table, "first_prefix", ""),
            Utils.lookupString(table, "second_prefix", ""),
            Utils.lookupArray(table, "excludes").toSeq.map(Utils.toString)
          )
        )

      case "bus" =>
        Some(
          parseBusConnection(
            first,
            dir,
            secondary,
            table
          )
        )

      case "logical" =>
        Some(UnconnectedLogical(first, dir, secondary))

      case _ =>
        error(s"$conntype is an unknown connection type")
        None
    }
  }

  /** Parses a bus connection from the connection table.
    *
    * @param first
    *   The name of the first component in the connection.
    * @param dir
    *   The direction of the connection.
    * @param secondary
    *   The name of the second component in the connection.
    * @param table
    *   The table of connection attributes.
    * @return
    *   An UnconnectedBus instance.
    */
  private def parseBusConnection(
      first: String,
      dir: ConnectionDirection,
      secondary: String,
      table: Map[String, Variant]
  ): UnconnectedBus = {
    val supplierBusName = Utils.lookupString(table, "bus_name", "")
    val consumerBusName =
      Utils.lookupString(table, "consumer_bus_name", supplierBusName)

    UnconnectedBus(
      first,
      dir,
      secondary,
      BusName.apply(Utils.lookupString(table, "bus_protocol", "internal")),
      BusName.apply(supplierBusName),
      BusName.apply(consumerBusName),
      Utils.lookupBoolean(table, "silent", or = false)
    )
  }

  /** Parses a bus connection from the BusConnectionConfig.
    *
    * @param first
    *   The name of the first component in the connection.
    * @param dir
    *   The direction of the connection.
    * @param secondary
    *   The name of the second component in the connection.
    * @param config
    *   The bus connection configuration.
    * @return
    *   An UnconnectedBus instance.
    */
  @annotation.targetName("parseBusConnectionConfigFromConfig")
  private def parseBusConnectionConfig(
      first: String,
      dir: ConnectionDirection,
      secondary: String,
      config: BusConnectionConfig
  ): UnconnectedBus = {

    UnconnectedBus(
      firstFullName = first,
      secondFullName = secondary,
      direction = dir,
      busProtocol = BusName(config.bus_protocol),
      supplierBusName = BusName(config.supplier_bus_name),
      consumerBusName = BusName(config.consumer_bus_name),
      silent = config.silent
    )
  }

  /** Parses connections from a list of connection configurations.
    *
    * @param configs
    *   The list of connection configurations to parse.
    * @return
    *   A sequence of UnconnectedLike instances.
    */
  def parseConnections(
      configs: List[ConnectionConfig]
  ): Seq[UnconnectedLike] = {
    configs.flatMap(parseConnectionConfig)
  }

  /** Parses a parameters connection from the parameters array.
    *
    * @param direction
    *   The direction of the connection.
    * @param secondFullName
    *   The name of the instance associated with the parameters.
    * @param parametersV
    *   An array of parameter variants.
    * @return
    *   An UnconnectedParameters instance.
    */
  def parseParametersConnection(
      direction: ConnectionDirection,
      secondFullName: String,
      parametersV: Array[Variant]
  ): UnconnectedParameters = {
    val parameters = parametersV.flatMap { v =>
      val table = Utils.toTable(v)
      if (!table.contains("name")) {
        error(s"parameter $v has no name field")
        None
      } else {
        val name = Utils.lookupString(table, "name", "NO_NAME")
        val paramType = if (table.contains("type")) {
          val typeStr = Utils.toString(table("type"))
          if (typeStr == "frequency") {
            if (!table.contains("value")) {
              error(s"frequency parameter $name has no value field")
              // Can't return None here, instead skip this parameter
              null
            } else {
              val freqStr = Utils.toString(table("value"))
              val freq = Utils.toFrequency(table("value"))
              FrequencyParameterType(freq)
            }
          } else {
            if (!table.contains("value")) {
              error(s"constant parameter $name has no value field")
              // Can't return None here, instead skip this parameter
              null
            } else {
              ConstantParameterType(table("value"))
            }
          }
        } else {
          if (!table.contains("value")) {
            error(s"parameter $name has no value field")
            // Can't return None here, instead skip this parameter
            null
          } else {
            ConstantParameterType(table("value"))
          }
        }
        // Only add parameter if paramType is not null
        if (paramType != null) Some(Parameter(name, paramType)) else None
      }
    }.toSeq

    // Create and return the UnconnectedParameters instance
    new UnconnectedParameters(direction, secondFullName, parameters)
  }

  /** Parses a parameters connection from the parameters configuration.
    *
    * @param direction
    *   The direction of the connection.
    * @param secondFullName
    *   The name of the instance associated with the parameters.
    * @param parameters
    *   A list of parameter configurations.
    * @return
    *   An UnconnectedParameters instance.
    */
  @annotation.targetName("parseParametersConnectionConfigFromConfig")
  def parseParametersConnectionConfig(
      direction: ConnectionDirection,
      secondFullName: String,
      parameters: List[ParameterConfig]
  ): UnconnectedParameters = {
    val parsedParameters = parameters.flatMap { param =>
      val name = param.name
      val paramType = param.`type`.getOrElse("") match {
        case "frequency" =>
          // Extract frequency value from Json
          val freqStr = param.value.asString.getOrElse("")
          // Convert to a Variant using Utils.stringToVariant
          val freqVariant = Utils.stringToVariant(freqStr)
          val freq = Utils.toFrequency(freqVariant)
          FrequencyParameterType(freq)
        case _ =>
          // Use the Json value directly as a string
          val valueStr = param.value.asString.getOrElse(param.value.toString)
          // Convert to a Variant using Utils.stringToVariant
          val valueVariant = Utils.stringToVariant(valueStr)
          ConstantParameterType(valueVariant)
      }
      Some(Parameter(name, paramType))
    }

    // Create and return the UnconnectedParameters instance
    new UnconnectedParameters(direction, secondFullName, parsedParameters)
  }

  /** Parses a port connection between two instances.
    *
    * @param cbp
    *   The connection priority.
    * @param fil
    *   The first instance location.
    * @param sil
    *   The second instance location.
    * @return
    *   A ConnectedPortGroup representing the connection.
    */
  def parsePortConnection(
      cbp: ConnectionPriority,
      fil: InstanceLoc,
      sil: InstanceLoc
  ): ConnectedPortGroup = {
    val fp = {
      if (fil.port.nonEmpty) fil.port.get
      else if (fil.isPin) {
        fil.instance.asInstanceOf[PinGroupInstance].constraint.ports.head
      } else if (fil.isClock) {
        HardwareBoundrary(fil.fullName, BitsDesc(1), WireDirection.Input)
      } else {
        if (fil.isGateware) error(s"${fil.fullName} unable to get port")
        HardwareBoundrary(fil.fullName, BitsDesc(1), WireDirection.Input)
      }
    }
    val sp = {
      if (sil.port.nonEmpty) sil.port.get
      else if (sil.isPin) {
        sil.instance.asInstanceOf[PinGroupInstance].constraint.ports.head
      } else if (sil.isClock) {
        HardwareBoundrary(sil.fullName, BitsDesc(1), WireDirection.Output)
      } else {
        if (sil.isGateware) error(s"${sil.fullName} unable to get port")
        HardwareBoundrary(sil.fullName, fp.width, WireDirection.Input)
      }
    }

    var firstDirection = fp.direction
    var secondDirection = sp.direction

    if (fp.direction != WireDirection.InOut) {
      if (sp.direction == WireDirection.InOut) secondDirection = fp.direction
    } else if (sp.direction != WireDirection.InOut)
      firstDirection = sp.direction

    val fport = fp.copy(direction = firstDirection)
    val sport = sp.copy(direction = secondDirection)

    val fmloc = InstanceLoc(fil.instance, Some(fport), fil.fullName)
    val fsloc = InstanceLoc(sil.instance, Some(sport), sil.fullName)

    ConnectedPortGroup(cbp, fmloc, ConnectionDirection.FirstToSecond, fsloc)
  }
}
