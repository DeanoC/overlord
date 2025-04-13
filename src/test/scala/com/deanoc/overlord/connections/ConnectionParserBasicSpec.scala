package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.{FirstToSecondConnection, SecondToFirstConnection, BiDirectionConnection}
import scala.jdk.CollectionConverters._

/**
 * Simple test suite for the ConnectionParser object.
 * 
 * This test suite focuses on the public interface of ConnectionParser
 * to ensure compatibility with future refactoring.
 */
class ConnectionParserBasicSpec extends AnyFlatSpec with Matchers {
  
  // Create a variant with the specified connection string and type
  private def createConnectionVariant(connectionStr: String, connType: String = "port"): Variant = {
    val map = new java.util.HashMap[String, Any]()
    map.put("type", connType)
    map.put("connection", connectionStr)
    Utils.toVariant(map)
  }
  
  "ConnectionParser.parseConnection" should "correctly parse a first-to-second connection" in {
    // Create variant with the connection string
    val variant = createConnectionVariant("device1 -> device2")
    val result = ConnectionParser.parseConnection(variant)
    
    // Verify we got a result
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[FirstToSecondConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "correctly parse a bidirectional connection" in {
    val variant = createConnectionVariant("device1 <-> device2")
    val result = ConnectionParser.parseConnection(variant)
    
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[BiDirectionConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "also accept <> as a bidirectional connection" in {
    val variant = createConnectionVariant("device1 <> device2")
    val result = ConnectionParser.parseConnection(variant)
    
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[BiDirectionConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "correctly parse a second-to-first connection" in {
    val variant = createConnectionVariant("device1 <- device2")
    val result = ConnectionParser.parseConnection(variant)
    
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[SecondToFirstConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "reject invalid connection formats" in {
    // Invalid direction symbol
    val invalidVariant = createConnectionVariant("device1 >> device2")
    ConnectionParser.parseConnection(invalidVariant) shouldBe None
    
    // Missing parts
    val missingVariant = createConnectionVariant("device1 ->")
    ConnectionParser.parseConnection(missingVariant) shouldBe None
    
    // Too many parts
    val tooManyVariant = createConnectionVariant("device1 -> device2 -> device3")
    ConnectionParser.parseConnection(tooManyVariant) shouldBe None
  }
  
  it should "handle different connection types correctly" in {
    // Bus connection
    val busMap = new java.util.HashMap[String, Any]()
    busMap.put("type", "bus")
    busMap.put("connection", "device1 -> device2")
    busMap.put("bus_protocol", "axi")
    val busVariant = Utils.toVariant(busMap)
    val busResult = ConnectionParser.parseConnection(busVariant)
    
    busResult shouldBe defined
    busResult.get shouldBe a[UnconnectedBus]
    
    // Logical connection
    val logicalVariant = createConnectionVariant("device1 -> device2", "logical")
    val logicalResult = ConnectionParser.parseConnection(logicalVariant)
    
    logicalResult shouldBe defined
    logicalResult.get shouldBe a[UnconnectedLogical]
    
    // Clock connection
    val clockVariant = createConnectionVariant("clock -> device", "clock")
    val clockResult = ConnectionParser.parseConnection(clockVariant)
    
    clockResult shouldBe defined
    clockResult.get shouldBe a[UnconnectedClock]
  }
  
  it should "reject connections with missing fields" in {
    // Missing type field
    val missingTypeMap = new java.util.HashMap[String, Any]()
    missingTypeMap.put("connection", "a -> b")
    val missingTypeVariant = Utils.toVariant(missingTypeMap)
    ConnectionParser.parseConnection(missingTypeVariant) shouldBe None
    
    // Missing connection field
    val missingConnMap = new java.util.HashMap[String, Any]()
    missingConnMap.put("type", "port")
    val missingConnVariant = Utils.toVariant(missingConnMap)
    ConnectionParser.parseConnection(missingConnVariant) shouldBe None
  }
}