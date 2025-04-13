package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.{FirstToSecondConnection, SecondToFirstConnection, BiDirectionConnection}
import scala.jdk.CollectionConverters._

/**
 * Simplified test suite for the Unconnected.apply method.
 * 
 * This test focuses on the basic parsing functionality of Unconnected.apply
 * with minimal dependencies on complex Variant creation.
 */
class SimpleUnconnectedApplySpec extends AnyFlatSpec with Matchers {
  
  // Simple test for port connection
  "Unconnected.apply" should "parse port connections" in {
    // Create a Java HashMap which is supported by Utils.toVariant
    val map = new java.util.HashMap[String, Any]()
    map.put("type", "port")
    map.put("connection", "device1 -> device2")
    
    // Use Utils.toVariant to create the Variant
    val variant = Utils.toVariant(map)
    
    // Parse the connection
    val result = Unconnected.apply(variant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedPort]
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[FirstToSecondConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "parse bus connections" in {
    // Create a Java HashMap with bus connection parameters
    val map = new java.util.HashMap[String, Any]()
    map.put("type", "bus")
    map.put("connection", "cpu -> memory")
    map.put("bus_protocol", "axi4")
    map.put("bus_name", "main_bus")
    map.put("consumer_bus_name", "mem_bus")
    
    // Convert to Variant
    val variant = Utils.toVariant(map)
    
    // Parse the connection
    val result = Unconnected.apply(variant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedBus]
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    assert(bus.direction.isInstanceOf[FirstToSecondConnection])
    bus.secondFullName shouldBe "memory"
    bus.busProtocol shouldBe "axi4"
    bus.supplierBusName shouldBe "main_bus"
    bus.consumerBusName shouldBe "mem_bus"
  }
  
  it should "parse logical connections" in {
    // Create a Java HashMap for logical connection
    val map = new java.util.HashMap[String, Any]()
    map.put("type", "logical")
    map.put("connection", "comp1 -> comp2")
    
    // Convert to Variant
    val variant = Utils.toVariant(map)
    
    // Parse the connection
    val result = Unconnected.apply(variant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedLogical]
    val logical = result.get.asInstanceOf[UnconnectedLogical]
    logical.firstFullName shouldBe "comp1"
    assert(logical.direction.isInstanceOf[FirstToSecondConnection])
    logical.secondFullName shouldBe "comp2"
  }
  
  it should "parse clock connections" in {
    // Create a Java HashMap for clock connection
    val map = new java.util.HashMap[String, Any]()
    map.put("type", "clock")
    map.put("connection", "clk -> device")
    
    // Convert to Variant
    val variant = Utils.toVariant(map)
    
    // Parse the connection
    val result = Unconnected.apply(variant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedClock]
    val clock = result.get.asInstanceOf[UnconnectedClock]
    clock.firstFullName shouldBe "clk"
    assert(clock.direction.isInstanceOf[FirstToSecondConnection])
    clock.secondFullName shouldBe "device"
  }
  
  it should "handle different connection directions" in {
    // Test bidirectional connection
    val biMap = new java.util.HashMap[String, Any]()
    biMap.put("type", "port")
    biMap.put("connection", "device1 <-> device2")
    
    val biResult = Unconnected.apply(Utils.toVariant(biMap))
    biResult shouldBe defined
    assert(biResult.get.asInstanceOf[UnconnectedPort].direction.isInstanceOf[BiDirectionConnection])
    
    // Test second-to-first connection
    val reverseMap = new java.util.HashMap[String, Any]()
    reverseMap.put("type", "port")
    reverseMap.put("connection", "device1 <- device2")
    
    val reverseResult = Unconnected.apply(Utils.toVariant(reverseMap))
    reverseResult shouldBe defined
    assert(reverseResult.get.asInstanceOf[UnconnectedPort].direction.isInstanceOf[SecondToFirstConnection])
  }
  
  it should "parse port group connections with prefixes" in {
    // Create a Java HashMap for port group connection with prefixes
    val map = new java.util.HashMap[String, Any]()
    map.put("type", "port_group")
    map.put("connection", "uart0 -> uart1")
    map.put("first_prefix", "tx_")
    map.put("second_prefix", "rx_")
    
    // Create a Java list for excludes
    val excludesList = new java.util.ArrayList[String]()
    excludesList.add("clock")
    excludesList.add("reset")
    map.put("excludes", excludesList)
    
    // Convert to Variant
    val variant = Utils.toVariant(map)
    
    // Parse the connection
    val result = Unconnected.apply(variant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedPortGroup]
    val portGroup = result.get.asInstanceOf[UnconnectedPortGroup]
    portGroup.firstFullName shouldBe "uart0"
    portGroup.secondFullName shouldBe "uart1"
    portGroup.first_prefix shouldBe "tx_"
    portGroup.second_prefix shouldBe "rx_"
    portGroup.excludes should contain allOf("clock", "reset")
  }
  
  it should "reject connections with missing fields" in {
    // Missing type field
    val missingTypeMap = new java.util.HashMap[String, Any]()
    missingTypeMap.put("connection", "a -> b")
    
    Unconnected.apply(Utils.toVariant(missingTypeMap)) shouldBe None
    
    // Missing connection field
    val missingConnMap = new java.util.HashMap[String, Any]()
    missingConnMap.put("type", "port")
    
    Unconnected.apply(Utils.toVariant(missingConnMap)) shouldBe None
  }
  
  it should "parse parameter connections" in {
    // Create a Java HashMap for parameter connection
    val map = new java.util.HashMap[String, Any]()
    map.put("type", "parameters")
    map.put("connection", "_ -> target_device")
    
    // Create parameters array
    val paramsList = new java.util.ArrayList[java.util.Map[String, Any]]()
    
    // Add parameter 1
    val param1 = new java.util.HashMap[String, Any]()
    param1.put("name", "frequency")
    param1.put("value", "100MHz")
    paramsList.add(param1)
    
    // Add parameter 2
    val param2 = new java.util.HashMap[String, Any]()
    param2.put("name", "voltage")
    param2.put("value", "3.3V")
    paramsList.add(param2)
    
    // Add parameters to the main map
    map.put("parameters", paramsList)
    
    // Convert to Variant
    val variant = Utils.toVariant(map)
    
    // Parse the connection
    val result = Unconnected.apply(variant)
    
    // Verify the result
    result shouldBe defined
    result.get shouldBe a[UnconnectedParameters]
    val params = result.get.asInstanceOf[UnconnectedParameters]
    params.instanceName shouldBe "target_device"
  }
}