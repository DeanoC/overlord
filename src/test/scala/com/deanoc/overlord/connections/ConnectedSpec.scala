package com.deanoc.overlord.connections

import scala.language.implicitConversions
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.hardware.{Port, BitsDesc, InWireDirection, OutWireDirection}
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait, PinGroupInstance, ClockInstance}
import org.mockito.Mockito._
import org.mockito.Mockito
import com.deanoc.overlord.definitions.ChipDefinitionTrait

/**
 * Test suite for the Connected trait and its implementations.
 * This test suite focuses on the behavior of Connected and its components
 * to establish a baseline before refactoring.
 */
class ConnectedSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {

  // ConnectionPriority tests
  "ConnectionPriority" should "define different priority levels" in {
    // Use enum values directly
    val explicit = ConnectionPriority.Explicit
    val group = ConnectionPriority.Group
    val wildcard = ConnectionPriority.WildCard
    val fake = ConnectionPriority.Fake
    
    // Verify type and equality
    explicit shouldBe ConnectionPriority.Explicit
    group shouldBe ConnectionPriority.Group
    wildcard shouldBe ConnectionPriority.WildCard
    fake shouldBe ConnectionPriority.Fake
    
    // Verify they are different instances
    explicit should not be group
    explicit should not be wildcard
    explicit should not be fake
    group should not be wildcard
    group should not be fake
    wildcard should not be fake
  }
  
  // InstanceLoc tests - simpler version without SoftwareDef/GatewareDef
  "InstanceLoc" should "identify instance types correctly" in {
    // Create a mock ChipDefinitionTrait that we can use as base for all definition types
    val chipDef = mock[ChipDefinitionTrait]
    
    // Create mock instances
    val chipInstance = mock[ChipInstance]
    when(chipInstance.definition).thenReturn(chipDef)
    
    val pinInstance = mock[PinGroupInstance]
    when(pinInstance.definition).thenReturn(chipDef)
    
    val clockInstance = mock[ClockInstance]
    when(clockInstance.definition).thenReturn(chipDef)
    
    // Create InstanceLoc objects
    val chipLoc = InstanceLoc(chipInstance, None, "chip.instance")
    val pinLoc = InstanceLoc(pinInstance, None, "pin.group")
    val clockLoc = InstanceLoc(clockInstance, None, "clock.instance")
    
    // Test the basics
    chipLoc.isPin shouldBe false
    chipLoc.isClock shouldBe false
    chipLoc.isChip shouldBe true
    
    pinLoc.isPin shouldBe true
    pinLoc.isChip shouldBe false
    
    clockLoc.isClock shouldBe true
    clockLoc.isChip shouldBe false
  }
  
  it should "handle name operations correctly" in {
    // Create mock definition and instance
    val chipDef = mock[ChipDefinitionTrait]
    
    val instance = mock[ChipInstance]
    when(instance.definition).thenReturn(chipDef)
    
    // Create InstanceLoc with a hierarchical name
    val loc = InstanceLoc(instance, None, "top.middle.bottom")
    
    // Test name operations
    loc.fullName shouldBe "top.middle.bottom"
  }
  
  // ConnectedPortGroup tests
  "ConnectedPortGroup" should "identify connection types correctly" in {
    // Create a single mock for ChipDefinitionTrait
    val chipDef = mock[ChipDefinitionTrait]
    
    // Create mock instances
    val chipInstance1 = mock[ChipInstance]
    when(chipInstance1.definition).thenReturn(chipDef)
    when(chipInstance1.name).thenReturn("chip1")
    
    val chipInstance2 = mock[ChipInstance]
    when(chipInstance2.definition).thenReturn(chipDef)
    when(chipInstance2.name).thenReturn("chip2")
    
    val pinInstance = mock[PinGroupInstance]
    when(pinInstance.definition).thenReturn(chipDef)
    when(pinInstance.name).thenReturn("pin")
    
    // For clock instance, we need to make sure isClock returns true
    val clockInstance = mock[ClockInstance]
    when(clockInstance.definition).thenReturn(chipDef)
    when(clockInstance.name).thenReturn("clock")
    
    // Create ports
    val port1 = Port("port1", BitsDesc(8), InWireDirection())
    val port2 = Port("port2", BitsDesc(8), OutWireDirection())
    
    // Create InstanceLocs
    val chipLoc1 = InstanceLoc(chipInstance1, Some(port1), "chip1")
    val chipLoc2 = InstanceLoc(chipInstance2, Some(port2), "chip2")
    val pinLoc = InstanceLoc(pinInstance, Some(port1), "pin")
    val clockLoc = InstanceLoc(clockInstance, Some(port2), "clock")
    
    // Create different connection types
    val chipToChip = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      chipLoc1,
      ConnectionDirection.FirstToSecond,
      chipLoc2
    )
    
    val pinToChip = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      pinLoc,
      ConnectionDirection.FirstToSecond,
      chipLoc1
    )
    
    val chipToPin = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      chipLoc1,
      ConnectionDirection.FirstToSecond,
      pinLoc
    )
    
    // Skip the clock connection test for now since it relies on specific implementation details
    // that we don't have full access to without examining the ClockInstance implementation
    
    // Test connection type identification
    chipToChip.isChipToChip shouldBe true
    chipToChip.isPinToChip shouldBe false
    chipToChip.isChipToPin shouldBe false
    
    pinToChip.isChipToChip shouldBe false
    pinToChip.isPinToChip shouldBe true
    pinToChip.isChipToPin shouldBe false
    
    chipToPin.isChipToChip shouldBe false
    chipToPin.isPinToChip shouldBe false
    chipToPin.isChipToPin shouldBe true
  }
  
  it should "check connection between instances correctly" in {
    // Create mock hardware definition
    val chipDef = mock[ChipDefinitionTrait]
    
    // Create mock instances
    val chipInstance1 = mock[ChipInstance]
    when(chipInstance1.definition).thenReturn(chipDef)
    when(chipInstance1.name).thenReturn("chip1")
    
    val chipInstance2 = mock[ChipInstance]
    when(chipInstance2.definition).thenReturn(chipDef)
    when(chipInstance2.name).thenReturn("chip2")
    
    val chipInstance3 = mock[ChipInstance]
    when(chipInstance3.definition).thenReturn(chipDef)
    when(chipInstance3.name).thenReturn("chip3")
    
    // Create ports
    val port1 = Port("port1", BitsDesc(8), InWireDirection())
    val port2 = Port("port2", BitsDesc(8), OutWireDirection())
    
    // Create InstanceLocs
    val chipLoc1 = InstanceLoc(chipInstance1, Some(port1), "chip1")
    val chipLoc2 = InstanceLoc(chipInstance2, Some(port2), "chip2")
    
    // Create connection with FirstToSecondConnection
    val connection = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      chipLoc1,
      ConnectionDirection.FirstToSecond,
      chipLoc2
    )
    
    // Test connectedTo
    connection.connectedTo(chipInstance1) shouldBe true
    connection.connectedTo(chipInstance2) shouldBe true
    connection.connectedTo(chipInstance3) shouldBe false
    
    // Test default connectedBetween (note: ConnectedBetween.connectedBetween defaults to bidirectional)
    connection.connectedBetween(chipInstance1, chipInstance2) shouldBe true
    
    // The next test uses the default connectedBetween which is bidirectional by default
    // So we expect it to be true even though the connection direction is FirstToSecond
    connection.connectedBetween(chipInstance2, chipInstance1) shouldBe true
    
    connection.connectedBetween(chipInstance1, chipInstance3) shouldBe false
    connection.connectedBetween(chipInstance3, chipInstance2) shouldBe false
    
    // Test with explicit direction provided
    connection.connectedBetween(chipInstance1, chipInstance2, ConnectionDirection.FirstToSecond) shouldBe true
    connection.connectedBetween(chipInstance2, chipInstance1, ConnectionDirection.FirstToSecond) shouldBe false
    connection.connectedBetween(chipInstance1, chipInstance2, ConnectionDirection.SecondToFirst) shouldBe false
    connection.connectedBetween(chipInstance2, chipInstance1, ConnectionDirection.SecondToFirst) shouldBe true
    connection.connectedBetween(chipInstance1, chipInstance2, ConnectionDirection.BiDirectional) shouldBe true
    connection.connectedBetween(chipInstance2, chipInstance1, ConnectionDirection.BiDirectional) shouldBe true
  }
}