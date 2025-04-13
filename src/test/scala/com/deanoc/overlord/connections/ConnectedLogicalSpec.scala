package com.deanoc.overlord.connections

import scala.language.implicitConversions
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.hardware.{Port, BitsDesc, InWireDirection, OutWireDirection}
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}
import org.mockito.Mockito._

/**
 * Test suite for the ConnectedLogical class.
 * This test suite focuses on establishing a baseline for the behavior of logical connections
 * before refactoring to Scala 3 features.
 */
class ConnectedLogicalSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  // Create mock definitions and instances for testing
  val mockHardwareDef = mock[HardwareDefinitionTrait]
  
  // Test basic ConnectedLogical properties
  "ConnectedLogical" should "store and retrieve basic properties correctly" in {
    // Create mock instances
    val srcInstance = mock[ChipInstance]
    when(srcInstance.definition).thenReturn(mockHardwareDef)
    when(srcInstance.name).thenReturn("source")
    
    val destInstance = mock[ChipInstance]
    when(destInstance.definition).thenReturn(mockHardwareDef)
    when(destInstance.name).thenReturn("destination")
    
    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "top.source")
    val destLoc = InstanceLoc(destInstance, None, "top.destination")
    
    // Create ConnectedLogical
    val logicalConn = ConnectedLogical(
      ConnectionPriority.Explicit,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    // Test basic properties
    logicalConn.connectionPriority shouldBe ConnectionPriority.Explicit
    logicalConn.first.get shouldBe srcLoc
    logicalConn.second.get shouldBe destLoc
    logicalConn.direction shouldBe a[FirstToSecondConnection]
    logicalConn.firstFullName shouldBe "top.source"
    logicalConn.secondFullName shouldBe "top.destination"
    logicalConn.firstLastName shouldBe "source"
    logicalConn.secondLastName shouldBe "destination"
  }
  
  it should "identify connection between instances correctly" in {
    // Create mock instances
    val srcInstance = mock[ChipInstance]
    when(srcInstance.definition).thenReturn(mockHardwareDef)
    when(srcInstance.name).thenReturn("source")
    
    val destInstance = mock[ChipInstance]
    when(destInstance.definition).thenReturn(mockHardwareDef)
    when(destInstance.name).thenReturn("destination")
    
    val otherInstance = mock[ChipInstance]
    when(otherInstance.definition).thenReturn(mockHardwareDef)
    when(otherInstance.name).thenReturn("other")
    
    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "top.source")
    val destLoc = InstanceLoc(destInstance, None, "top.destination")
    
    // Create ConnectedLogical with FirstToSecond direction
    val forwardLogical = ConnectedLogical(
      ConnectionPriority.Explicit,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    // Create ConnectedLogical with bidirectional connection
    val biLogical = ConnectedLogical(
      ConnectionPriority.Explicit,
      srcLoc,
      BiDirectionConnection(),
      destLoc
    )
    
    // Test instance connection checks
    forwardLogical.connectedTo(srcInstance) shouldBe true
    forwardLogical.connectedTo(destInstance) shouldBe true
    forwardLogical.connectedTo(otherInstance) shouldBe false
    
    // Test directional connections
    forwardLogical.connectedBetween(srcInstance, destInstance) shouldBe true // default is bidirectional
    forwardLogical.connectedBetween(destInstance, srcInstance) shouldBe true // default is bidirectional
    
    // Test with explicit directions
    forwardLogical.connectedBetween(srcInstance, destInstance, FirstToSecondConnection()) shouldBe true
    forwardLogical.connectedBetween(destInstance, srcInstance, FirstToSecondConnection()) shouldBe false
    forwardLogical.connectedBetween(srcInstance, destInstance, SecondToFirstConnection()) shouldBe false
    forwardLogical.connectedBetween(destInstance, srcInstance, SecondToFirstConnection()) shouldBe true
    forwardLogical.connectedBetween(srcInstance, destInstance, BiDirectionConnection()) shouldBe true
    forwardLogical.connectedBetween(destInstance, srcInstance, BiDirectionConnection()) shouldBe true
    
    // Test bidirectional logical connections
    biLogical.connectedBetween(srcInstance, destInstance, FirstToSecondConnection()) shouldBe true
    biLogical.connectedBetween(destInstance, srcInstance, FirstToSecondConnection()) shouldBe false
    biLogical.connectedBetween(srcInstance, destInstance, SecondToFirstConnection()) shouldBe false
    biLogical.connectedBetween(destInstance, srcInstance, SecondToFirstConnection()) shouldBe true
    biLogical.connectedBetween(srcInstance, destInstance, BiDirectionConnection()) shouldBe true
    biLogical.connectedBetween(destInstance, srcInstance, BiDirectionConnection()) shouldBe true
  }
  
  it should "correctly identify chip-to-chip connections" in {
    // Create mock instances
    val srcInstance = mock[ChipInstance]
    when(srcInstance.definition).thenReturn(mockHardwareDef)
    when(srcInstance.name).thenReturn("source")
    
    val destInstance = mock[ChipInstance]
    when(destInstance.definition).thenReturn(mockHardwareDef)
    when(destInstance.name).thenReturn("destination")
    
    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "top.source")
    val destLoc = InstanceLoc(destInstance, None, "top.destination")
    
    // Create ConnectedLogical
    val logicalConn = ConnectedLogical(
      ConnectionPriority.Explicit,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    // Test connection type identification
    logicalConn.isChipToChip shouldBe true
    logicalConn.isPinToChip shouldBe false
    logicalConn.isChipToPin shouldBe false
    logicalConn.isClock shouldBe false
  }
  
  it should "handle name operations correctly" in {
    // Create mock instances
    val srcInstance = mock[ChipInstance]
    when(srcInstance.definition).thenReturn(mockHardwareDef)
    when(srcInstance.name).thenReturn("source")
    
    val destInstance = mock[ChipInstance]
    when(destInstance.definition).thenReturn(mockHardwareDef)
    when(destInstance.name).thenReturn("destination")
    
    // Create InstanceLocs with hierarchical names
    val srcLoc = InstanceLoc(srcInstance, None, "soc.cpu.core")
    val destLoc = InstanceLoc(destInstance, None, "soc.memory.controller")
    
    // Create ConnectedLogical
    val logicalConn = ConnectedLogical(
      ConnectionPriority.Explicit,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    // Test name operations
    logicalConn.firstFullName shouldBe "soc.cpu.core"
    logicalConn.secondFullName shouldBe "soc.memory.controller"
    logicalConn.firstLastName shouldBe "core"
    logicalConn.secondLastName shouldBe "controller"
    logicalConn.firstHeadName shouldBe "soc"
    logicalConn.secondHeadName shouldBe "soc"
  }
  
  it should "handle different connection priorities" in {
    // Create mock instances
    val srcInstance = mock[ChipInstance]
    when(srcInstance.definition).thenReturn(mockHardwareDef)
    val destInstance = mock[ChipInstance]
    when(destInstance.definition).thenReturn(mockHardwareDef)
    
    // Create InstanceLocs
    val srcLoc = InstanceLoc(srcInstance, None, "src")
    val destLoc = InstanceLoc(destInstance, None, "dest")
    
    // Create connections with different priorities
    val explicitConn = ConnectedLogical(
      ConnectionPriority.Explicit,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    val groupConn = ConnectedLogical(
      ConnectionPriority.Group,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    val wildcardConn = ConnectedLogical(
      ConnectionPriority.WildCard,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    val fakeConn = ConnectedLogical(
      ConnectionPriority.Fake,
      srcLoc,
      FirstToSecondConnection(),
      destLoc
    )
    
    // Verify connection priorities
    explicitConn.connectionPriority shouldBe ConnectionPriority.Explicit
    groupConn.connectionPriority shouldBe ConnectionPriority.Group
    wildcardConn.connectionPriority shouldBe ConnectionPriority.WildCard
    fakeConn.connectionPriority shouldBe ConnectionPriority.Fake
  }
}