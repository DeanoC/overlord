package com.deanoc.overlord.Connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.{
  BiDirectionConnection, 
  FirstToSecondConnection, 
  SecondToFirstConnection
}
import com.deanoc.overlord.utils.{Utils, Variant}
import org.scalatestplus.mockito.MockitoSugar

/**
 * Test suite specifically for the Unconnected.apply method.
 *
 * This test suite verifies the parsing behavior of the Unconnected.apply method,
 * focusing on how it handles different connection types and edge cases.
 */
class UnconnectedApplySpec extends AnyFlatSpec with Matchers with MockitoSugar {
  
  // Helper method to create a variant for testing
  private def createConnectionVariant(
    connType: String,
    connection: String,
    additionalParams: Map[String, Any] = Map.empty
  ): Variant = {
    // Create a Java HashMap which is supported by Utils.toVariant
    val map = new java.util.HashMap[String, Any]()
    
    // Add the required fields
    map.put("type", connType)
    map.put("connection", connection)
    
    // Add any additional parameters
    additionalParams.foreach { case (key, value) =>
      map.put(key, value)
    }
    
    // Convert to Variant
    Utils.toVariant(map)
  }
  
  "Unconnected.apply" should "parse port connections" in {
    // Create a simple port connection variant
    val portVariant = createConnectionVariant("port", "device1 -> device2")
    
    // Parse the connection
    val result = Unconnected.apply(portVariant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedPort]
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    port.direction shouldBe a[FirstToSecondConnection]
    port.secondFullName shouldBe "device2"
  }
  
  it should "parse bus connections" in {
    // Create additional parameters as raw values
    val busParams = Map(
      "bus_protocol" -> "axi4",
      "bus_name" -> "main_bus",
      "consumer_bus_name" -> "mem_bus"
    )
    
    // Create a bus connection variant
    val busVariant = createConnectionVariant("bus", "cpu -> memory", busParams)
    
    // Parse the connection
    val result = Unconnected.apply(busVariant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedBus]
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    bus.direction shouldBe a[FirstToSecondConnection]
    bus.secondFullName shouldBe "memory"
    bus.busProtocol shouldBe "axi4"
    bus.supplierBusName shouldBe "main_bus"
    bus.consumerBusName shouldBe "mem_bus"
  }

  it should "parse logical connections" in {
    // Create a logical connection variant
    val logicalVariant = createConnectionVariant("logical", "comp1 -> comp2")
    
    val result = Unconnected.apply(logicalVariant)
    
    result shouldBe defined
    result.get shouldBe a[UnconnectedLogical]
    val logical = result.get.asInstanceOf[UnconnectedLogical]
    logical.firstFullName shouldBe "comp1"
    logical.direction shouldBe a[FirstToSecondConnection]
    logical.secondFullName shouldBe "comp2"
  }
  
  it should "parse clock connections" in {
    // Create a clock connection variant
    val clockVariant = createConnectionVariant("clock", "clk -> device")
    
    // Parse the connection
    val result = Unconnected.apply(clockVariant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedClock]
    val clock = result.get.asInstanceOf[UnconnectedClock]
    clock.firstFullName shouldBe "clk"
    clock.direction shouldBe a[FirstToSecondConnection]
    clock.secondFullName shouldBe "device"
  }
  
  it should "parse port group connections with additional parameters" in {
    // Create a port group variant with prefixes and excludes
    val excludesList = new java.util.ArrayList[String]()
    excludesList.add("clock")
    excludesList.add("reset")
    
    val portGroupParams = Map(
      "first_prefix" -> "tx_",
      "second_prefix" -> "rx_",
      "excludes" -> excludesList
    )
    
    val portGroupVariant = createConnectionVariant("port_group", "uart0 -> uart1", portGroupParams)
    
    // Parse the connection
    val result = Unconnected.apply(portGroupVariant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedPortGroup]
    val portGroup = result.get.asInstanceOf[UnconnectedPortGroup]
    portGroup.firstFullName shouldBe "uart0"
    portGroup.direction shouldBe a[FirstToSecondConnection]
    portGroup.secondFullName shouldBe "uart1"
    portGroup.first_prefix shouldBe "tx_"
    portGroup.second_prefix shouldBe "rx_"
    portGroup.excludes should contain allOf("clock", "reset")
  }
  
  it should "handle parameters connections" in {
    // Create parameters array
    val paramsList = new java.util.ArrayList[java.util.Map[String, Any]]()
    
    // Add parameter 1
    val param1 = new java.util.HashMap[String, Any]()
    param1.put("name", "param1")
    param1.put("value", "value1")
    paramsList.add(param1)
    
    // Add parameter 2
    val param2 = new java.util.HashMap[String, Any]()
    param2.put("name", "param2")
    param2.put("value", "value2")
    paramsList.add(param2)
    
    // Create the parameters connection variant
    val parametersVariant = createConnectionVariant(
      "parameters",
      "_ -> target",
      Map("parameters" -> paramsList)
    )
    
    // Parse the connection
    val result = Unconnected.apply(parametersVariant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedParameters]
    val params = result.get.asInstanceOf[UnconnectedParameters]
    params.direction shouldBe a[FirstToSecondConnection]
    params.instanceName shouldBe "target"
  }
  
  it should "handle different connection directions" in {
    // Test bi-directional connections
    val biDirectionalVariant = createConnectionVariant("port", "device1 <-> device2")
    val biDirResult = Unconnected.apply(biDirectionalVariant)
    biDirResult shouldBe defined
    biDirResult.get.asInstanceOf[UnconnectedPort].direction shouldBe a[BiDirectionConnection]
    
    // Test second-to-first connections
    val reverseVariant = createConnectionVariant("port", "device1 <- device2")
    val reverseResult = Unconnected.apply(reverseVariant)
    reverseResult shouldBe defined
    reverseResult.get.asInstanceOf[UnconnectedPort].direction shouldBe a[SecondToFirstConnection]
  }
  
  it should "reject missing required fields" in {
    // Missing type field
    val missingTypeMap = new java.util.HashMap[String, Any]()
    missingTypeMap.put("connection", "a -> b")
    val missingType = Utils.toVariant(missingTypeMap)
    Unconnected.apply(missingType) shouldBe None
    
    // Missing connection field
    val missingConnectionMap = new java.util.HashMap[String, Any]()
    missingConnectionMap.put("type", "port")
    val missingConnection = Utils.toVariant(missingConnectionMap)
    Unconnected.apply(missingConnection) shouldBe None
  }
  
  it should "reject invalid connection format" in {
    // Invalid direction symbol
    val invalidDirection = createConnectionVariant("port", "a => b")
    Unconnected.apply(invalidDirection) shouldBe None
    
    // Invalid parts count
    val invalidParts = createConnectionVariant("port", "a -> b -> c")
    Unconnected.apply(invalidParts) shouldBe None
  }
  
  it should "reject unknown connection types" in {
    val unknownType = createConnectionVariant("unknown_type", "a -> b")
    Unconnected.apply(unknownType) shouldBe None
  }
  
  it should "reject parameters connections with invalid first value" in {
    // For parameters, first value must be "_"
    // Create parameter entry
    val paramsList = new java.util.ArrayList[java.util.Map[String, Any]]()
    
    // Add a parameter
    val param = new java.util.HashMap[String, Any]()
    param.put("name", "param")
    param.put("value", "val")
    paramsList.add(param)
    
    // Create the invalid parameters connection variant
    val invalidParams = createConnectionVariant(
      "parameters",
      "device1 -> device2", // should be "_ -> device2"
      Map("parameters" -> paramsList)
    )
    
    Unconnected.apply(invalidParams) shouldBe None
  }
}