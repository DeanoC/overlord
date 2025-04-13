package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}

/**
 * Simple test suite for the ConnectionParser object.
 * 
 * This test suite focuses on the public interface of ConnectionParser
 * to ensure compatibility with future refactoring.
 */
class ConnectionParserBasicSpec extends AnyFlatSpec with Matchers with SilentLogger {
  
  /**
   * Helper method to create a test Variant for connection testing.
   *
   * @param connType The connection type (e.g., "port", "bus")
   * @param connection The connection string (e.g., "device1 -> device2")
   * @return A Variant containing the connection information
   */
  private def createTestVariant(connType: String, connection: String): Variant = {
    val map = new java.util.HashMap[String, Any]()
    map.put("type", connType)
    map.put("connection", connection)
    Utils.toVariant(map)
  }
  
  "ConnectionParser.parseConnection" should "correctly parse a first-to-second connection" in {
    // Create variant with the connection string
    val variant = createTestVariant("port", "device1 -> device2")
    val result = ConnectionParser.parseConnection(variant)
    
    // Verify we got a result
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[FirstToSecondConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "correctly parse a bidirectional connection" in {
    val variant = createTestVariant("port", "device1 <-> device2")
    val result = ConnectionParser.parseConnection(variant)
    
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[BiDirectionConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "also accept <> as a bidirectional connection" in {
    val variant = createTestVariant("port", "device1 <> device2")
    val result = ConnectionParser.parseConnection(variant)
    
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[BiDirectionConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "correctly parse a second-to-first connection" in {
    val variant = createTestVariant("port", "device1 <- device2")
    val result = ConnectionParser.parseConnection(variant)
    
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPort]
    port.firstFullName shouldBe "device1"
    assert(port.direction.isInstanceOf[SecondToFirstConnection])
    port.secondFullName shouldBe "device2"
  }
  
  it should "reject invalid connection formats" in {
    withSilentLogs {
      // Test with invalid operator
      val invalidOperatorVariant = createTestVariant("port", "device1 >> device2")
      ConnectionParser.parseConnection(invalidOperatorVariant) shouldBe None
      
      // Test with incomplete connection (missing second part)
      val incompleteVariant = createTestVariant("port", "device1 ->")
      ConnectionParser.parseConnection(incompleteVariant) shouldBe None
      
      // Test with too many parts
      val tooManyPartsVariant = createTestVariant("port", "device1 -> device2 -> device3")
      ConnectionParser.parseConnection(tooManyPartsVariant) shouldBe None
    }
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
    val logicalVariant = createTestVariant("logical", "device1 -> device2")
    val logicalResult = ConnectionParser.parseConnection(logicalVariant)
    
    logicalResult shouldBe defined
    logicalResult.get shouldBe a[UnconnectedLogical]
    
    // Clock connection
    val clockVariant = createTestVariant("clock", "clock -> device")
    val clockResult = ConnectionParser.parseConnection(clockVariant)
    
    clockResult shouldBe defined
    clockResult.get shouldBe a[UnconnectedClock]
  }
  
  it should "reject connections with missing fields" in {
    withSilentLogs {
      // Create a connection with missing type field
      val missingTypeMap = new java.util.HashMap[String, Any]()
      missingTypeMap.put("connection", "a -> b")
      val missingTypeVariant = Utils.toVariant(missingTypeMap)
      ConnectionParser.parseConnection(missingTypeVariant) shouldBe None
      
      // Create a connection with missing connection field
      val missingConnMap = new java.util.HashMap[String, Any]()
      missingConnMap.put("type", "port")
      val missingConnVariant = Utils.toVariant(missingConnMap)
      ConnectionParser.parseConnection(missingConnVariant) shouldBe None
    }
  }
}