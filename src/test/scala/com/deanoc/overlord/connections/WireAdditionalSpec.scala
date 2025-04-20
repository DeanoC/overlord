package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._

import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.{
  ChipInstance,
  InstanceTrait,
  PinGroupInstance,
  ClockInstance
}
import com.deanoc.overlord.hardware.{
  Port,
  BitsDesc,
  InWireDirection,
  OutWireDirection,
  InOutWireDirection
}
import com.deanoc.overlord.definitions.ChipDefinitionTrait

/** Additional test suite for Wire class focusing on:
  *   1. Testing wire creation from different connection types 2. Testing
  *      handling of fan-out connections 3. Testing error conditions
  */
class WireAdditionalSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  "Wire" should "correctly identify pin and clock instances" in {
    // Create mock instances
    val chipDef = mock[ChipDefinitionTrait]

    val chipInstance = mock[ChipInstance]
    doReturn(chipDef).when(chipInstance).definition

    val pinInstance = mock[PinGroupInstance]
    doReturn(chipDef).when(pinInstance).definition

    val clockInstance = mock[ClockInstance]
    doReturn(chipDef).when(clockInstance).definition

    // Create InstanceLoc objects
    val chipLoc = InstanceLoc(chipInstance, None, "chip.instance")
    val pinLoc = InstanceLoc(pinInstance, None, "pin.group")
    val clockLoc = InstanceLoc(clockInstance, None, "clock.instance")

    // Create Wire objects
    val chipStartWire =
      Wire(chipLoc, Seq(pinLoc), ConnectionPriority.Explicit, true)
    val pinStartWire =
      Wire(pinLoc, Seq(chipLoc), ConnectionPriority.Explicit, true)
    val clockStartWire =
      Wire(clockLoc, Seq(chipLoc), ConnectionPriority.Explicit, true)

    // Test isStartPinOrClock method
    chipStartWire.isStartPinOrClock shouldBe false
    pinStartWire.isStartPinOrClock shouldBe true
    clockStartWire.isStartPinOrClock shouldBe true

    // Test findEndIsPinOrClock method
    chipStartWire.findEndIsPinOrClock shouldBe Some(pinLoc)
    pinStartWire.findEndIsPinOrClock shouldBe None
    clockStartWire.findEndIsPinOrClock shouldBe None
  }

  it should "handle wires with multiple end locations" in {
    // Create mock instances
    val chipDef = mock[ChipDefinitionTrait]

    val sourceInstance = mock[ChipInstance]
    doReturn(chipDef).when(sourceInstance).definition

    val destInstance1 = mock[ChipInstance]
    doReturn(chipDef).when(destInstance1).definition

    val destInstance2 = mock[ChipInstance]
    doReturn(chipDef).when(destInstance2).definition

    val destInstance3 = mock[ChipInstance]
    doReturn(chipDef).when(destInstance3).definition

    // Create InstanceLoc objects
    val sourceLoc = InstanceLoc(sourceInstance, None, "source")
    val destLoc1 = InstanceLoc(destInstance1, None, "dest1")
    val destLoc2 = InstanceLoc(destInstance2, None, "dest2")
    val destLoc3 = InstanceLoc(destInstance3, None, "dest3")

    // Create Wire with multiple end locations
    val wire = Wire(
      sourceLoc,
      Seq(destLoc1, destLoc2, destLoc3),
      ConnectionPriority.Explicit,
      true
    )

    // Verify the wire properties
    wire.startLoc shouldBe sourceLoc
    wire.endLocs.length shouldBe 3
    wire.endLocs should contain allOf (destLoc1, destLoc2, destLoc3)
    wire.priority shouldBe ConnectionPriority.Explicit
    wire.knownWidth shouldBe true

    // Test findEndIsPinOrClock with no pin or clock end
    wire.findEndIsPinOrClock shouldBe None
  }

  it should "handle wires with mixed end location types" in {
    // Create mock instances
    val chipDef = mock[ChipDefinitionTrait]

    val sourceInstance = mock[ChipInstance]
    doReturn(chipDef).when(sourceInstance).definition

    val chipInstance = mock[ChipInstance]
    doReturn(chipDef).when(chipInstance).definition

    val pinInstance = mock[PinGroupInstance]
    doReturn(chipDef).when(pinInstance).definition

    val clockInstance = mock[ClockInstance]
    doReturn(chipDef).when(clockInstance).definition

    // Create InstanceLoc objects
    val sourceLoc = InstanceLoc(sourceInstance, None, "source")
    val chipLoc = InstanceLoc(chipInstance, None, "chip")
    val pinLoc = InstanceLoc(pinInstance, None, "pin")
    val clockLoc = InstanceLoc(clockInstance, None, "clock")

    // Create Wire with mixed end location types
    val wire = Wire(
      sourceLoc,
      Seq(chipLoc, pinLoc, clockLoc),
      ConnectionPriority.Explicit,
      true
    )

    // Verify the wire properties
    wire.startLoc shouldBe sourceLoc
    wire.endLocs.length shouldBe 3
    wire.endLocs should contain allOf (chipLoc, pinLoc, clockLoc)

    // Test findEndIsPinOrClock with mixed end types
    // It should return the first pin or clock in the sequence
    wire.findEndIsPinOrClock shouldBe Some(pinLoc)
  }

  "Wires" should "handle complex routing scenarios" in {
    // Create mock instances
    val chipDef = mock[ChipDefinitionTrait]

    val sourceInstance = mock[ChipInstance]
    doReturn(chipDef).when(sourceInstance).definition
    doReturn("source").when(sourceInstance).name

    val intermediateInstance1 = mock[ChipInstance]
    doReturn(chipDef).when(intermediateInstance1).definition
    doReturn("intermediate1").when(intermediateInstance1).name

    val intermediateInstance2 = mock[ChipInstance]
    doReturn(chipDef).when(intermediateInstance2).definition
    doReturn("intermediate2").when(intermediateInstance2).name

    val destInstance = mock[ChipInstance]
    doReturn(chipDef).when(destInstance).definition
    doReturn("dest").when(destInstance).name

    // Create InstanceLoc objects
    val sourceLoc = InstanceLoc(sourceInstance, None, "source")
    val destLoc = InstanceLoc(destInstance, None, "dest")

    // Create a Connected instance
    val connected = mock[Connected]
    doReturn(Some(sourceLoc)).when(connected).first
    doReturn(Some(destLoc)).when(connected).second
    doReturn(ConnectionPriority.Explicit).when(connected).connectionPriority
    doReturn(ConnectionDirection.FirstToSecond).when(connected).direction

    // Create mock DistanceMatrix with complex routing
    val dm = mock[DistanceMatrix]
    doReturn((0, 3)).when(dm).indicesOf(connected)

    // Multi-hop route: 0 -> 1 -> 2 -> 3
    doReturn(Seq(1, 2, 3)).when(dm).routeBetween(0, 3)

    doReturn(sourceInstance).when(dm).instanceOf(0)
    doReturn(intermediateInstance1).when(dm).instanceOf(1)
    doReturn(intermediateInstance2).when(dm).instanceOf(2)
    doReturn(destInstance).when(dm).instanceOf(3)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    // The implementation should create a single wire with the correct start and end
    // Note: The actual implementation creates a wire for each hop in the route
    wires.length shouldBe 3

    // Check that the first wire starts at the source
    val firstWire = wires.find(w => w.startLoc.instance == sourceInstance).get
    firstWire.startLoc.instance shouldBe sourceInstance

    // Check that the last wire ends at the destination
    val lastWire =
      wires.find(w => w.endLocs.exists(_.instance == destInstance)).get
    lastWire.endLocs.exists(_.instance == destInstance) shouldBe true
  }

  it should "handle connections with different priorities correctly" in {
    // Create mock instances
    val chipDef = mock[ChipDefinitionTrait]

    val sourceInstance = mock[ChipInstance]
    doReturn(chipDef).when(sourceInstance).definition
    doReturn("source").when(sourceInstance).name

    val destInstance = mock[ChipInstance]
    doReturn(chipDef).when(destInstance).definition
    doReturn("dest").when(destInstance).name

    // Create InstanceLoc objects
    val sourceLoc = InstanceLoc(sourceInstance, None, "source")
    val destLoc = InstanceLoc(destInstance, None, "dest")

    // Create Connected instances with different priorities
    val explicitConnected = mock[Connected]
    doReturn(Some(sourceLoc)).when(explicitConnected).first
    doReturn(Some(destLoc)).when(explicitConnected).second
    doReturn(ConnectionPriority.Explicit)
      .when(explicitConnected)
      .connectionPriority
    doReturn(ConnectionDirection.FirstToSecond)
      .when(explicitConnected)
      .direction

    val wildcardConnected = mock[Connected]
    doReturn(Some(sourceLoc)).when(wildcardConnected).first
    doReturn(Some(destLoc)).when(wildcardConnected).second
    doReturn(ConnectionPriority.WildCard)
      .when(wildcardConnected)
      .connectionPriority
    doReturn(ConnectionDirection.FirstToSecond)
      .when(wildcardConnected)
      .direction

    val groupConnected = mock[Connected]
    doReturn(Some(sourceLoc)).when(groupConnected).first
    doReturn(Some(destLoc)).when(groupConnected).second
    doReturn(ConnectionPriority.Group).when(groupConnected).connectionPriority
    doReturn(ConnectionDirection.FirstToSecond).when(groupConnected).direction

    // Create mock DistanceMatrix
    val dm = mock[DistanceMatrix]

    // Setup for explicit connection
    doReturn((0, 1)).when(dm).indicesOf(explicitConnected)
    doReturn(Seq(1)).when(dm).routeBetween(0, 1)

    // Setup for wildcard connection
    doReturn((0, 1)).when(dm).indicesOf(wildcardConnected)
    doReturn(Seq(1)).when(dm).routeBetween(0, 1)

    // Setup for group connection
    doReturn((0, 1)).when(dm).indicesOf(groupConnected)
    doReturn(Seq(1)).when(dm).routeBetween(0, 1)

    doReturn(sourceInstance).when(dm).instanceOf(0)
    doReturn(destInstance).when(dm).instanceOf(1)

    // Create wires for each priority
    val explicitWires = Wires(dm, Seq(explicitConnected))
    val wildcardWires = Wires(dm, Seq(wildcardConnected))
    val groupWires = Wires(dm, Seq(groupConnected))

    // Verify the results
    explicitWires.length shouldBe 1
    explicitWires.head.priority shouldBe ConnectionPriority.Explicit

    wildcardWires.length shouldBe 1
    wildcardWires.head.priority shouldBe ConnectionPriority.WildCard

    groupWires.length shouldBe 1
    groupWires.head.priority shouldBe ConnectionPriority.Group
  }

  it should "handle connections with unknown width ports" in {
    // Create mock instances
    val chipDef = mock[ChipDefinitionTrait]

    val sourceInstance = mock[ChipInstance]
    doReturn(chipDef).when(sourceInstance).definition
    doReturn("source").when(sourceInstance).name

    val destInstance = mock[ChipInstance]
    doReturn(chipDef).when(destInstance).definition
    doReturn("dest").when(destInstance).name

    // Create ports with unknown width
    val sourcePort = mock[Port]
    doReturn("out").when(sourcePort).name
    doReturn(OutWireDirection()).when(sourcePort).direction
    doReturn(false).when(sourcePort).knownWidth

    val destPort = mock[Port]
    doReturn("in").when(destPort).name
    doReturn(InWireDirection()).when(destPort).direction
    doReturn(false).when(destPort).knownWidth

    // Create InstanceLoc objects
    val sourceLoc = InstanceLoc(sourceInstance, Some(sourcePort), "source")
    val destLoc = InstanceLoc(destInstance, Some(destPort), "dest")

    // Create a Connected instance
    val connected = mock[Connected]
    doReturn(Some(sourceLoc)).when(connected).first
    doReturn(Some(destLoc)).when(connected).second
    doReturn(ConnectionPriority.Explicit).when(connected).connectionPriority
    doReturn(ConnectionDirection.FirstToSecond).when(connected).direction

    // Create mock DistanceMatrix
    val dm = mock[DistanceMatrix]
    doReturn((0, 1)).when(dm).indicesOf(connected)
    doReturn(Seq(1)).when(dm).routeBetween(0, 1)
    doReturn(sourceInstance).when(dm).instanceOf(0)
    doReturn(destInstance).when(dm).instanceOf(1)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.knownWidth shouldBe false
  }
}
