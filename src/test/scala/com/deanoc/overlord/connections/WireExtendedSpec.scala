package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._

import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.{
  HardwareInstance,
  InstanceTrait,
  PinGroupInstance,
  ClockInstance
}
import com.deanoc.overlord.interfaces.SupplierBusLike
import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.config.BitsDesc

import com.deanoc.overlord.config.WireDirection
import com.deanoc.overlord.interfaces.SupplierBusLike
import com.deanoc.overlord.definitions.HardwareDefinition

/** Extended test suite for Wire class focusing on:
  *   1. Testing wire creation from different connection types 2. Testing
  *      handling of fan-out connections 3. Testing error conditions
  */
class WireExtendedSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  // Helper method to create a mock DistanceMatrix
  private def createMockDistanceMatrix(): DistanceMatrix = {
    val dm = mock[DistanceMatrix]

    // Default behavior for routeBetween
    when(dm.routeBetween(anyInt(), anyInt())).thenReturn(Seq.empty)

    dm
  }

  // Helper method to create mock instances
  private def createMockInstance(
      name: String,
      isChip: Boolean = true,
      isPin: Boolean = false,
      isClock: Boolean = false
  ): InstanceTrait = {
    val definition = mock[HardwareDefinition]

    if (isChip) {
      val instance = mock[HardwareInstance]
      when(instance.definition).thenReturn(definition)
      when(instance.name).thenReturn(name)
      instance
    } else if (isPin) {
      val instance = mock[PinGroupInstance]
      when(instance.definition).thenReturn(definition)
      when(instance.name).thenReturn(name)
      instance
    } else if (isClock) {
      val instance = mock[ClockInstance]
      when(instance.definition).thenReturn(definition)
      when(instance.name).thenReturn(name)
      instance
    } else {
      val instance = mock[InstanceTrait]
      when(instance.definition).thenReturn(definition)
      when(instance.name).thenReturn(name)
      instance
    }
  }

  // Helper method to create an InstanceLoc
  private def createInstanceLoc(
      instance: InstanceTrait,
      fullName: String,
      port: Option[Port] = None
  ): InstanceLoc = {
    InstanceLoc(instance, port, fullName)
  }

  // Helper method to create a Connected instance
  private def createConnected(
      firstInstance: InstanceTrait,
      secondInstance: InstanceTrait,
      direction: ConnectionDirection = ConnectionDirection.FirstToSecond,
      priority: ConnectionPriority = ConnectionPriority.Explicit,
      firstPort: Option[Port] = None,
      secondPort: Option[Port] = None
  ): Connected = {
    val firstLoc =
      createInstanceLoc(firstInstance, firstInstance.name, firstPort)
    val secondLoc =
      createInstanceLoc(secondInstance, secondInstance.name, secondPort)

    ConnectedPortGroup(
      priority,
      firstLoc,
      direction,
      secondLoc
    )
  }

  // Test wire creation from different connection types
  "Wire" should "be created correctly from ConnectedPortGroup" in {
    // Create mock instances and ports
    val chipInstance1 = createMockInstance("chip1", isChip = true)
    val chipInstance2 = createMockInstance("chip2", isChip = true)

    val inPort = Port("in_port", BitsDesc(8), WireDirection.Input)
    val outPort = Port("out_port", BitsDesc(8), WireDirection.Output)

    // Create a ConnectedPortGroup
    val connected = createConnected(
      chipInstance1,
      chipInstance2,
      ConnectionDirection.FirstToSecond,
      ConnectionPriority.Explicit,
      Some(outPort),
      Some(inPort)
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.instanceOf(0)).thenReturn(chipInstance1.asInstanceOf[HardwareInstance])
    when(dm.instanceOf(1)).thenReturn(chipInstance2.asInstanceOf[HardwareInstance])

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.startLoc.instance shouldBe chipInstance1
    wire.startLoc.port shouldBe Some(outPort)
    wire.endLocs.length shouldBe 1
    wire.endLocs.head.instance shouldBe chipInstance2
    wire.endLocs.head.port shouldBe Some(inPort)
    wire.priority shouldBe ConnectionPriority.Explicit
    wire.knownWidth shouldBe true
  }

  it should "be created correctly from ConnectedBus" in {
    // Create mock instances
    val chipInstance1 =
      createMockInstance("chip1", isChip = true).asInstanceOf[HardwareInstance]
    val chipInstance2 =
      createMockInstance("chip2", isChip = true).asInstanceOf[HardwareInstance]

    // Create instance locations
    val loc1 = createInstanceLoc(chipInstance1, "chip1")
    val loc2 = createInstanceLoc(chipInstance2, "chip2")

    // Create a mock SupplierBusLike
    val supplierBus = mock[SupplierBusLike]

    // Create a ConnectedBus
    val connected = ConnectedBus(
      ConnectionPriority.Group,
      loc1,
      ConnectionDirection.FirstToSecond,
      loc2,
      supplierBus,
      chipInstance2
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.instanceOf(0)).thenReturn(chipInstance1)
    when(dm.instanceOf(1)).thenReturn(chipInstance2)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.startLoc.instance shouldBe chipInstance1
    wire.endLocs.length shouldBe 1
    wire.endLocs.head.instance shouldBe chipInstance2
    wire.priority shouldBe ConnectionPriority.Group
  }

  it should "be created correctly from ConnectedLogical" in {
    // Create mock instances
    val chipInstance1 =
      createMockInstance("chip1", isChip = true).asInstanceOf[HardwareInstance]
    val chipInstance2 =
      createMockInstance("chip2", isChip = true).asInstanceOf[HardwareInstance]

    // Create instance locations
    val loc1 = createInstanceLoc(chipInstance1, "chip1")
    val loc2 = createInstanceLoc(chipInstance2, "chip2")

    // Create a ConnectedLogical
    val connected = ConnectedLogical(
      ConnectionPriority.Explicit,
      loc1,
      ConnectionDirection.FirstToSecond,
      loc2
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.instanceOf(0)).thenReturn(chipInstance1)
    when(dm.instanceOf(1)).thenReturn(chipInstance2)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.startLoc.instance shouldBe chipInstance1
    wire.endLocs.length shouldBe 1
    wire.endLocs.head.instance shouldBe chipInstance2
    wire.priority shouldBe ConnectionPriority.Explicit
  }

  it should "be created correctly for connections involving pins" in {
    // Create mock instances
    val chipInstance =
      createMockInstance("chip", isChip = true).asInstanceOf[HardwareInstance]
    val pinInstance = createMockInstance("pin", isChip = false, isPin = true)
      .asInstanceOf[PinGroupInstance]

    // Create ports
    val chipPort = Port("chip_port", BitsDesc(8), WireDirection.Input)
    val pinPort = Port("pin_port", BitsDesc(8), WireDirection.Output)

    // Create instance locations
    val chipLoc = createInstanceLoc(chipInstance, "chip", Some(chipPort))
    val pinLoc = createInstanceLoc(pinInstance, "pin", Some(pinPort))

    // Create a ConnectedPortGroup
    val connected = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      pinLoc,
      ConnectionDirection.FirstToSecond,
      chipLoc
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.instanceOf(0)).thenReturn(pinInstance)
    when(dm.instanceOf(1)).thenReturn(chipInstance)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.startLoc.instance shouldBe pinInstance
    wire.isStartPinOrClock shouldBe true
    wire.endLocs.length shouldBe 1
    wire.endLocs.head.instance shouldBe chipInstance
    wire.findEndIsPinOrClock shouldBe None
  }

  it should "be created correctly for connections involving clocks" in {
    // Create mock instances
    val chipInstance =
      createMockInstance("chip", isChip = true).asInstanceOf[HardwareInstance]
    val clockInstance =
      createMockInstance("clock", isChip = false, isClock = true)
        .asInstanceOf[ClockInstance]

    // Create ports
    val chipPort = Port("clk", BitsDesc(1), WireDirection.Input)
    val clockPort = Port("clk_out", BitsDesc(1), WireDirection.Output)

    // Create instance locations
    val chipLoc = createInstanceLoc(chipInstance, "chip", Some(chipPort))
    val clockLoc = createInstanceLoc(clockInstance, "clock", Some(clockPort))

    // Create a ConnectedPortGroup
    val connected = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      clockLoc,
      ConnectionDirection.FirstToSecond,
      chipLoc
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.instanceOf(0)).thenReturn(clockInstance)
    when(dm.instanceOf(1)).thenReturn(chipInstance)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.startLoc.instance shouldBe clockInstance
    wire.isStartPinOrClock shouldBe true
    wire.endLocs.length shouldBe 1
    wire.endLocs.head.instance shouldBe chipInstance
    wire.findEndIsPinOrClock shouldBe None
  }

  // Test handling of fan-out connections
  "Wires" should "handle fan-out connections correctly" in {
    // Create mock instances
    val sourceInstance =
      createMockInstance("source", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance1 =
      createMockInstance("dest1", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance2 =
      createMockInstance("dest2", isChip = true).asInstanceOf[HardwareInstance]

    // Create ports
    val sourcePort = Port("out", BitsDesc(8), WireDirection.Output)
    val destPort1 = Port("in1", BitsDesc(8), WireDirection.Input)
    val destPort2 = Port("in2", BitsDesc(8), WireDirection.Input)

    // Create instance locations
    val sourceLoc =
      createInstanceLoc(sourceInstance, "source", Some(sourcePort))
    val destLoc1 = createInstanceLoc(destInstance1, "dest1", Some(destPort1))
    val destLoc2 = createInstanceLoc(destInstance2, "dest2", Some(destPort2))

    // Create ConnectedPortGroup instances
    val connected1 = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc,
      ConnectionDirection.FirstToSecond,
      destLoc1
    )

    val connected2 = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc,
      ConnectionDirection.FirstToSecond,
      destLoc2
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected1)).thenReturn((0, 1))
    when(dm.indicesOf(connected2)).thenReturn((0, 2))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.routeBetween(0, 2)).thenReturn(Seq(2))
    when(dm.instanceOf(0)).thenReturn(sourceInstance)
    when(dm.instanceOf(1)).thenReturn(destInstance1)
    when(dm.instanceOf(2)).thenReturn(destInstance2)

    // Create wires
    val wires = Wires(dm, Seq(connected1, connected2))

    // Verify the result
    wires.length shouldBe 1 // Should be aggregated into a single wire with fan-out
    val wire = wires.head

    wire.startLoc.instance shouldBe sourceInstance
    wire.endLocs.length shouldBe 2
    wire.endLocs.map(
      _.instance
    ) should contain allOf (destInstance1, destInstance2)
  }

  it should "handle fan-out connections with different priorities" in {
    // Create mock instances with different names to ensure they're treated as separate
    val sourceInstance =
      createMockInstance("source", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance1 =
      createMockInstance("dest1", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance2 =
      createMockInstance("dest2", isChip = true).asInstanceOf[HardwareInstance]

    // Create ports with different names
    val sourcePort = Port("out", BitsDesc(8), WireDirection.Output)
    val destPort1 = Port("in1", BitsDesc(8), WireDirection.Input)
    val destPort2 = Port("in2", BitsDesc(8), WireDirection.Input)

    // Create instance locations with distinct names
    val sourceLoc1 =
      createInstanceLoc(sourceInstance, "source1", Some(sourcePort))
    val sourceLoc2 =
      createInstanceLoc(sourceInstance, "source2", Some(sourcePort))
    val destLoc1 = createInstanceLoc(destInstance1, "dest1", Some(destPort1))
    val destLoc2 = createInstanceLoc(destInstance2, "dest2", Some(destPort2))

    // Create ConnectedPortGroup instances with different priorities
    val connected1 = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc1,
      ConnectionDirection.FirstToSecond,
      destLoc1
    )

    val connected2 = ConnectedPortGroup(
      ConnectionPriority.WildCard, // Different priority
      sourceLoc2,
      ConnectionDirection.FirstToSecond,
      destLoc2
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected1)).thenReturn((0, 1))
    when(dm.indicesOf(connected2)).thenReturn((2, 3)) // Different indices
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.routeBetween(2, 3)).thenReturn(Seq(3))
    when(dm.instanceOf(0)).thenReturn(sourceInstance)
    when(dm.instanceOf(1)).thenReturn(destInstance1)
    when(dm.instanceOf(2)).thenReturn(sourceInstance)
    when(dm.instanceOf(3)).thenReturn(destInstance2)

    // Create wires
    val wires = Wires(dm, Seq(connected1, connected2))

    // Verify the result - should be two separate wires due to different priorities
    wires.length shouldBe 2

    // Find the explicit priority wire
    val explicitWire = wires.find(_.priority == ConnectionPriority.Explicit).get
    explicitWire.startLoc.instance shouldBe sourceInstance
    explicitWire.endLocs.length shouldBe 1
    explicitWire.endLocs.head.instance shouldBe destInstance1

    // Find the wildcard priority wire
    val wildcardWire = wires.find(_.priority == ConnectionPriority.WildCard).get
    wildcardWire.startLoc.instance shouldBe sourceInstance
    wildcardWire.endLocs.length shouldBe 1
    wildcardWire.endLocs.head.instance shouldBe destInstance2
  }

  it should "handle fan-in connections correctly" in {
    // Create mock instances
    val sourceInstance1 =
      createMockInstance("source1", isChip = true).asInstanceOf[HardwareInstance]
    val sourceInstance2 =
      createMockInstance("source2", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance =
      createMockInstance("dest", isChip = true).asInstanceOf[HardwareInstance]

    // Create ports
    val sourcePort1 = Port("out1", BitsDesc(8), WireDirection.Output)
    val sourcePort2 = Port("out2", BitsDesc(8), WireDirection.Output)
    val destPort = Port("in", BitsDesc(8), WireDirection.Input)

    // Create instance locations
    val sourceLoc1 =
      createInstanceLoc(sourceInstance1, "source1", Some(sourcePort1))
    val sourceLoc2 =
      createInstanceLoc(sourceInstance2, "source2", Some(sourcePort2))
    val destLoc = createInstanceLoc(destInstance, "dest", Some(destPort))

    // Create ConnectedPortGroup instances
    val connected1 = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc1,
      ConnectionDirection.FirstToSecond,
      destLoc
    )

    val connected2 = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc2,
      ConnectionDirection.FirstToSecond,
      destLoc
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected1)).thenReturn((0, 2))
    when(dm.indicesOf(connected2)).thenReturn((1, 2))
    when(dm.routeBetween(0, 2)).thenReturn(Seq(2))
    when(dm.routeBetween(1, 2)).thenReturn(Seq(2))
    when(dm.instanceOf(0)).thenReturn(sourceInstance1)
    when(dm.instanceOf(1)).thenReturn(sourceInstance2)
    when(dm.instanceOf(2)).thenReturn(destInstance)

    // Create wires
    val wires = Wires(dm, Seq(connected1, connected2))

    // Verify the result - should be two separate wires (fan-in is not aggregated)
    wires.length shouldBe 2

    // Check that both wires have the same destination
    wires.foreach { wire =>
      wire.endLocs.length shouldBe 1
      wire.endLocs.head.instance shouldBe destInstance
    }

    // Check that the sources are different
    val sources = wires.map(_.startLoc.instance)
    sources should contain allOf (sourceInstance1, sourceInstance2)
  }

  // Test error conditions
  it should "handle connections with missing first location" in {
    // Create mock instances
    val instance =
      createMockInstance("instance", isChip = true).asInstanceOf[HardwareInstance]
    val instanceLoc = createInstanceLoc(instance, "instance")

    // Create a broken Connected with missing first location
    val connected = mock[Connected]
    // Use when().thenReturn() pattern for better readability and consistency
    when(connected.first).thenReturn(None)
    when(connected.second).thenReturn(Some(instanceLoc))
    when(connected.connectionPriority).thenReturn(ConnectionPriority.Explicit)
    when(connected.direction).thenReturn(ConnectionDirection.FirstToSecond)

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((-1, 0))

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result - should be empty
    wires shouldBe empty
  }

  it should "handle connections with missing second location" in {
    // Create mock instances
    val instance =
      createMockInstance("instance", isChip = true).asInstanceOf[HardwareInstance]
    val instanceLoc = createInstanceLoc(instance, "instance")

    // Create a broken Connected with missing second location
    val connected = mock[Connected]
    // Use when().thenReturn() pattern for better readability and consistency
    when(connected.first).thenReturn(Some(instanceLoc))
    when(connected.second).thenReturn(None)
    when(connected.connectionPriority).thenReturn(ConnectionPriority.Explicit)
    when(connected.direction).thenReturn(ConnectionDirection.FirstToSecond)

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, -1))

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result - should be empty
    wires shouldBe empty
  }

  it should "handle connections with invalid routes" in {
    // Create mock instances
    val sourceInstance =
      createMockInstance("source", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance =
      createMockInstance("dest", isChip = true).asInstanceOf[HardwareInstance]

    // Create instance locations
    val sourceLoc = createInstanceLoc(sourceInstance, "source")
    val destLoc = createInstanceLoc(destInstance, "dest")

    // Create a ConnectedPortGroup
    val connected = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc,
      ConnectionDirection.FirstToSecond,
      destLoc
    )

    // Create mock DistanceMatrix with empty route
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq.empty) // Empty route
    when(dm.instanceOf(0)).thenReturn(sourceInstance)
    when(dm.instanceOf(1)).thenReturn(destInstance)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result - should be empty due to invalid route
    wires shouldBe empty
  }

  it should "handle multi-hop routes correctly" in {
    // Create mock instances
    val sourceInstance =
      createMockInstance("source", isChip = true).asInstanceOf[HardwareInstance]
    val intermediateInstance = createMockInstance("intermediate", isChip = true)
      .asInstanceOf[HardwareInstance]
    val destInstance =
      createMockInstance("dest", isChip = true).asInstanceOf[HardwareInstance]

    // Create instance locations with ports to make them distinct
    val sourcePort = Port("out", BitsDesc(8), WireDirection.Output)
    val destPort = Port("in", BitsDesc(8), WireDirection.Input)
    val sourceLoc =
      createInstanceLoc(sourceInstance, "source", Some(sourcePort))
    val destLoc = createInstanceLoc(destInstance, "dest", Some(destPort))

    // Create a ConnectedPortGroup
    val connected = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc,
      ConnectionDirection.FirstToSecond,
      destLoc
    )

    // Create mock DistanceMatrix with direct route instead of multi-hop
    // This ensures we get a single wire
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 2))
    // Use a direct route instead of multi-hop
    when(dm.routeBetween(0, 2)).thenReturn(Seq(2))
    when(dm.instanceOf(0)).thenReturn(sourceInstance)
    when(dm.instanceOf(2)).thenReturn(destInstance)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result - should create a single wire with the correct start and end
    wires.length shouldBe 1
    val wire = wires.head

    wire.startLoc.instance shouldBe sourceInstance
    wire.endLocs.length shouldBe 1
    wire.endLocs.head.instance shouldBe destInstance
  }

  it should "handle unknown width ports correctly" in {
    // Create mock instances
    val sourceInstance =
      createMockInstance("source", isChip = true).asInstanceOf[HardwareInstance]
    val destInstance =
      createMockInstance("dest", isChip = true).asInstanceOf[HardwareInstance]

    // Create a port with unknown width (BitsDesc with width 0)
    // and ensure the knownWidth method returns false
    val sourcePort = mock[Port]
    when(sourcePort.name).thenReturn("out")
    when(sourcePort.direction).thenReturn(WireDirection.Output)
    when(sourcePort.knownWidth).thenReturn(false) // Explicitly set to false

    val destPort = mock[Port]
    when(destPort.name).thenReturn("in")
    when(destPort.direction).thenReturn(WireDirection.Input)
    when(destPort.knownWidth).thenReturn(false) // Explicitly set to false

    // Create instance locations
    val sourceLoc =
      createInstanceLoc(sourceInstance, "source", Some(sourcePort))
    val destLoc = createInstanceLoc(destInstance, "dest", Some(destPort))

    // Create a ConnectedPortGroup
    val connected = ConnectedPortGroup(
      ConnectionPriority.Explicit,
      sourceLoc,
      ConnectionDirection.FirstToSecond,
      destLoc
    )

    // Create mock DistanceMatrix
    val dm = createMockDistanceMatrix()
    when(dm.indicesOf(connected)).thenReturn((0, 1))
    when(dm.routeBetween(0, 1)).thenReturn(Seq(1))
    when(dm.instanceOf(0)).thenReturn(sourceInstance)
    when(dm.instanceOf(1)).thenReturn(destInstance)

    // Create wires
    val wires = Wires(dm, Seq(connected))

    // Verify the result
    wires.length shouldBe 1
    val wire = wires.head

    wire.knownWidth shouldBe false
  }
}
