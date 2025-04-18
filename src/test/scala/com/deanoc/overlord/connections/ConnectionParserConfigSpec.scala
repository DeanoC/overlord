package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.config._
import io.circe.Json

/**
 * Test suite for the ConnectionParser with type-safe configuration classes.
 * 
 * This test suite demonstrates how to use the new type-safe configuration classes
 * for testing connection parsing, showing the improved testability of the refactored code.
 */
class ConnectionParserConfigSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {
  
  "ConnectionParser.parseConnections" should "parse a list of connection configurations" in {
    // Create a list of connection configurations
    val configs = List(
      PortConnectionConfig(
        connection = "device1 -> device2",
        `type` = "port"
      ),
      BusConnectionConfig(
        connection = "cpu -> ram",
        `type` = "bus",
        bus_protocol = Some("axi"),
        bus_name = Some("data_bus")
      ),
      ClockConnectionConfig(
        connection = "clock -> device",
        `type` = "clock"
      )
    )
    
    // Parse the connections
    val connections = ConnectionParser.parseConnections(configs)
    
    // Verify the result
    connections should have size 3
    connections(0) shouldBe a[UnconnectedPortGroup]
    connections(1) shouldBe a[UnconnectedBus]
    connections(2) shouldBe a[UnconnectedClock]
  }
  
  "ConnectionParser.parseConnectionConfig" should "parse different connection types correctly" in {
    // Test port connection
    val portConfig = PortConnectionConfig(
      connection = "device1 -> device2",
      `type` = "port"
    )
    
    val portResult = ConnectionParser.parseConnectionConfig(portConfig)
    portResult shouldBe defined
    portResult.get shouldBe a[UnconnectedPortGroup]
    val port = portResult.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    port.secondFullName shouldBe "device2"
    port.direction shouldBe ConnectionDirection.FirstToSecond
    
    // Test bus connection
    val busConfig = BusConnectionConfig(
      connection = "cpu -> ram",
      `type` = "bus",
      bus_protocol = Some("axi"),
      bus_name = Some("data_bus"),
      consumer_bus_name = Some("mem_bus"),
      silent = Some(true)
    )
    
    val busResult = ConnectionParser.parseConnectionConfig(busConfig)
    busResult shouldBe defined
    busResult.get shouldBe a[UnconnectedBus]
    val bus = busResult.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    bus.secondFullName shouldBe "ram"
    bus.direction shouldBe ConnectionDirection.FirstToSecond
    bus.protocol.name shouldBe "axi"
    bus.supplierBusName.name shouldBe "data_bus"
    bus.consumerBusName.name shouldBe "mem_bus"
    bus.silent shouldBe true
    
    // Test clock connection
    val clockConfig = ClockConnectionConfig(
      connection = "clock -> device",
      `type` = "clock"
    )
    
    val clockResult = ConnectionParser.parseConnectionConfig(clockConfig)
    clockResult shouldBe defined
    clockResult.get shouldBe a[UnconnectedClock]
    val clock = clockResult.get.asInstanceOf[UnconnectedClock]
    clock.firstFullName shouldBe "clock"
    clock.secondFullName shouldBe "device"
    clock.direction shouldBe ConnectionDirection.FirstToSecond
    
    // Test logical connection
    val logicalConfig = LogicalConnectionConfig(
      connection = "device1 <-> device2",
      `type` = "logical"
    )
    
    val logicalResult = ConnectionParser.parseConnectionConfig(logicalConfig)
    logicalResult shouldBe defined
    logicalResult.get shouldBe a[UnconnectedLogical]
    val logical = logicalResult.get.asInstanceOf[UnconnectedLogical]
    logical.firstFullName shouldBe "device1"
    logical.secondFullName shouldBe "device2"
    logical.direction shouldBe ConnectionDirection.BiDirectional
  }
  
  it should "parse port group connections with prefixes and excludes" in {
    val portGroupConfig = PortGroupConnectionConfig(
      connection = "device1 -> device2",
      `type` = "port_group",
      first_prefix = Some("prefix1_"),
      second_prefix = Some("prefix2_"),
      excludes = Some(List("exclude1", "exclude2"))
    )
    
    val result = ConnectionParser.parseConnectionConfig(portGroupConfig)
    
    result shouldBe defined
    val portGroup = result.get.asInstanceOf[UnconnectedPortGroup]
    portGroup.firstFullName shouldBe "device1"
    portGroup.secondFullName shouldBe "device2"
    portGroup.firstPrefix shouldBe "prefix1_"
    portGroup.secondPrefix shouldBe "prefix2_"
    portGroup.excludes should contain allOf("exclude1", "exclude2")
  }
  
  it should "parse parameters connections" in {
    val parametersConfig = ParametersConnectionConfig(
      connection = "_ -> device",
      `type` = "parameters",
      parameters = List(
        ParameterConfig(
          name = "param1",
          value = Json.fromString("value1")
        ),
        ParameterConfig(
          name = "param2",
          value = Json.fromInt(42)
        ),
        ParameterConfig(
          name = "frequency",
          value = Json.fromString("100MHz"),
          `type` = Some("frequency")
        )
      )
    )
    
    val result = ConnectionParser.parseConnectionConfig(parametersConfig)
    
    result shouldBe defined
    val params = result.get.asInstanceOf[UnconnectedParameters]
    params.secondFullName shouldBe "device"
    params.parameters should have size 3
    
    // Check the parameter names
    val paramNames = params.parameters.map(_.name)
    paramNames should contain allOf("param1", "param2", "frequency")
    
    // Find the frequency parameter
    val freqParam = params.parameters.find(_.name == "frequency").get
    freqParam.paramType shouldBe a[FrequencyParameterType]
  }
  
  it should "reject invalid connection formats" in {
    withSilentLogs {
      // Test with invalid operator
      val invalidOperatorConfig = PortConnectionConfig(
        connection = "device1 >> device2",
        `type` = "port"
      )
      ConnectionParser.parseConnectionConfig(invalidOperatorConfig) shouldBe None
      
      // Test with incomplete connection (missing second part)
      val incompleteConfig = PortConnectionConfig(
        connection = "device1 ->",
        `type` = "port"
      )
      ConnectionParser.parseConnectionConfig(incompleteConfig) shouldBe None
      
      // Test with too many parts
      val tooManyPartsConfig = PortConnectionConfig(
        connection = "device1 -> device2 -> device3",
        `type` = "port"
      )
      ConnectionParser.parseConnectionConfig(tooManyPartsConfig) shouldBe None
    }
  }
  
  "ConnectionParser.parseParametersConnectionConfig" should "parse parameters correctly" in {
    // Create parameters with different types
    val parameters = List(
      ParameterConfig(
        name = "string_param",
        value = Json.fromString("string_value")
      ),
      ParameterConfig(
        name = "int_param",
        value = Json.fromInt(42)
      ),
      ParameterConfig(
        name = "bool_param",
        value = Json.fromBoolean(true)
      ),
      ParameterConfig(
        name = "freq_param",
        value = Json.fromString("100MHz"),
        `type` = Some("frequency")
      )
    )
    
    // Parse the parameters
    val unconnectedParams = ConnectionParser.parseParametersConnectionConfig(
      ConnectionDirection.FirstToSecond,
      "target_device",
      parameters
    )
    
    // Verify the result
    unconnectedParams.secondFullName shouldBe "target_device"
    unconnectedParams.parameters should have size 4
    
    // Check each parameter
    val stringParam = unconnectedParams.parameters.find(_.name == "string_param").get
    stringParam.paramType shouldBe a[ConstantParameterType]
    
    val intParam = unconnectedParams.parameters.find(_.name == "int_param").get
    intParam.paramType shouldBe a[ConstantParameterType]
    
    val boolParam = unconnectedParams.parameters.find(_.name == "bool_param").get
    boolParam.paramType shouldBe a[ConstantParameterType]
    
    val freqParam = unconnectedParams.parameters.find(_.name == "freq_param").get
    freqParam.paramType shouldBe a[FrequencyParameterType]
    val freqValue = freqParam.paramType.asInstanceOf[FrequencyParameterType].frequency
    freqValue shouldBe 100000000 // 100MHz in Hz
  }
  
  "ConnectionParser.parseBusConnectionConfig" should "parse bus connections correctly" in {
    // Create a bus connection config
    val busConfig = BusConnectionConfig(
      connection = "cpu -> ram",
      `type` = "bus",
      bus_protocol = Some("axi4"),
      bus_name = Some("master_bus"),
      consumer_bus_name = Some("slave_bus"),
      silent = Some(true)
    )
    
    // Parse the bus connection
    val result = ConnectionParser.parseConnectionConfig(busConfig)
    
    // Verify the result
    result shouldBe defined
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    bus.secondFullName shouldBe "ram"
    bus.direction shouldBe ConnectionDirection.FirstToSecond
    bus.protocol.name shouldBe "axi4"
    bus.supplierBusName.name shouldBe "master_bus"
    bus.consumerBusName.name shouldBe "slave_bus"
    bus.silent shouldBe true
  }
  
  it should "use default values when optional fields are not provided" in {
    // Create a minimal bus connection config
    val minimalBusConfig = BusConnectionConfig(
      connection = "cpu -> ram",
      `type` = "bus"
    )
    
    // Parse the bus connection
    val result = ConnectionParser.parseConnectionConfig(minimalBusConfig)
    
    // Verify the result
    result shouldBe defined
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    bus.secondFullName shouldBe "ram"
    bus.protocol.name shouldBe "internal" // Default protocol
    bus.supplierBusName.name shouldBe "" // Default bus name
    bus.consumerBusName.name shouldBe "" // Default consumer bus name
    bus.silent shouldBe false // Default silent value
  }
  
  it should "use supplier bus name for consumer when consumer bus name is not provided" in {
    // Create a bus connection config with supplier bus name but no consumer bus name
    val busConfig = BusConnectionConfig(
      connection = "cpu -> ram",
      `type` = "bus",
      bus_name = Some("data_bus")
    )
    
    // Parse the bus connection
    val result = ConnectionParser.parseConnectionConfig(busConfig)
    
    // Verify the result
    result shouldBe defined
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.supplierBusName.name shouldBe "data_bus"
    bus.consumerBusName.name shouldBe "data_bus" // Should use supplier bus name
  }
}