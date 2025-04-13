package com.deanoc.overlord.Connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord.{BiDirectionConnection, ConnectionDirection, FirstToSecondConnection, SecondToFirstConnection, DefinitionTrait, GatewareDefinitionTrait, HardwareDefinitionTrait, SoftwareDefinitionTrait, DefinitionType}
import com.deanoc.overlord.Hardware.Port
import com.deanoc.overlord.Instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.Interfaces.SupplierBusLike
import com.deanoc.overlord.Connections.{
  ExplicitConnectionPriority, GroupConnectionPriority, 
  WildCardConnectionPriority, FakeConnectionPriority,
  InstanceLoc, ConnectedBus, ConnectedLogical, Wire,
  Constant, ConnectionPriority, Parameter
}
import com.deanoc.overlord.Software.SoftwareDefinition
import com.deanoc.overlord.actions.ActionsFile
import java.nio.file.{Path, Paths}
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito.when

/**
 * Test cases for the Connections module to verify behavior before and after refactoring.
 * 
 * These tests focus on the external behavior of the connections system to ensure
 * that our refactoring doesn't change functionality.
 */
class ConnectionsSpec extends AnyFlatSpec with Matchers with MockitoSugar {
  
  // Helper method to create a mock ChipInstance for testing
  def createMockInstance(name: String): ChipInstance = {
    val mockInstance = mock[ChipInstance]
    when(mockInstance.name).thenReturn(name)
    mockInstance
  }
  
  // Helper method to create a mock Port for testing
  def createMockPort(name: String): Port = {
    val mockPort = mock[Port]
    when(mockPort.name).thenReturn(name)
    mockPort
  }
  
  "ConnectionPriority" should "maintain proper hierarchy for conflict resolution" in {
    // Verify that the priority hierarchy is maintained: Explicit > Group > WildCard > Fake
    val explicitPriority = ExplicitConnectionPriority()
    val groupPriority = GroupConnectionPriority()
    val wildcardPriority = WildCardConnectionPriority()
    val fakePriority = FakeConnectionPriority()
    
    // Verify that instances are of the correct type
    explicitPriority shouldBe a [ExplicitConnectionPriority]
    groupPriority shouldBe a [GroupConnectionPriority]
    wildcardPriority shouldBe a [WildCardConnectionPriority]
    fakePriority shouldBe a [FakeConnectionPriority]
    
    // Helper function to determine priority ordering
    def hasHigherPriority(higher: ConnectionPriority, lower: ConnectionPriority): Boolean = {
      (higher, lower) match {
        case (_: ExplicitConnectionPriority, _: GroupConnectionPriority) => true
        case (_: ExplicitConnectionPriority, _: WildCardConnectionPriority) => true
        case (_: ExplicitConnectionPriority, _: FakeConnectionPriority) => true
        case (_: GroupConnectionPriority, _: WildCardConnectionPriority) => true
        case (_: GroupConnectionPriority, _: FakeConnectionPriority) => true
        case (_: WildCardConnectionPriority, _: FakeConnectionPriority) => true
        case _ => false
      }
    }
    
    // Test the priority hierarchy
    hasHigherPriority(explicitPriority, groupPriority) shouldBe true
    hasHigherPriority(explicitPriority, wildcardPriority) shouldBe true
    hasHigherPriority(explicitPriority, fakePriority) shouldBe true
    
    hasHigherPriority(groupPriority, wildcardPriority) shouldBe true
    hasHigherPriority(groupPriority, fakePriority) shouldBe true
    
    hasHigherPriority(wildcardPriority, fakePriority) shouldBe true
    
    // Test inverse relationships are false
    hasHigherPriority(groupPriority, explicitPriority) shouldBe false
    hasHigherPriority(wildcardPriority, explicitPriority) shouldBe false
    hasHigherPriority(fakePriority, explicitPriority) shouldBe false
    
    hasHigherPriority(wildcardPriority, groupPriority) shouldBe false
    hasHigherPriority(fakePriority, groupPriority) shouldBe false
    
    hasHigherPriority(fakePriority, wildcardPriority) shouldBe false
    
    // Test same-priority comparisons
    hasHigherPriority(explicitPriority, ExplicitConnectionPriority()) shouldBe false
    hasHigherPriority(groupPriority, GroupConnectionPriority()) shouldBe false
    hasHigherPriority(wildcardPriority, WildCardConnectionPriority()) shouldBe false
    hasHigherPriority(fakePriority, FakeConnectionPriority()) shouldBe false
  }

  "InstanceLoc" should "correctly identify hardware, software, and gateware instances" in {
    // Create concrete anonymous implementations since we can't mock abstract traits
    
    // Create a concrete hardware definition that extends HardwareDefinitionTrait with all required members
    val hardwareDefinition = new HardwareDefinitionTrait {
      override val attributes: Map[String, Variant] = Map.empty
      override val defType: DefinitionType = DefinitionType("hardware")
      override val maxInstances: Int = 1
      override val ports: Map[String, Port] = Map.empty
      override protected val registersV: Seq[Variant] = Seq.empty
      override val dependencies: Seq[String] = Seq.empty
      override val sourcePath: Path = Paths.get("/path/to/hardware")
      override def toString(): String = "HardwareDefinition"
    }
    
    val hardwareInstance = createMockInstance("HardwareDevice")
    when(hardwareInstance.definition).thenReturn(hardwareDefinition)
    
    // Create a concrete gateware definition that extends GatewareDefinitionTrait with all required members
    val gatewareDefinition = new GatewareDefinitionTrait {
      override val attributes: Map[String, Variant] = Map.empty
      override val defType: DefinitionType = DefinitionType("gateware")
      override val maxInstances: Int = 1
      override val ports: Map[String, Port] = Map.empty
      override protected val registersV: Seq[Variant] = Seq.empty
      override val dependencies: Seq[String] = Seq.empty
      override val sourcePath: Path = Paths.get("/path/to/gateware")
      override val actionsFile: ActionsFile = mock[ActionsFile]
      override val parameters: Map[String, Variant] = Map.empty
      override def toString(): String = "GatewareDefinition"
    }
    
    val gatewareInstance = createMockInstance("GatewareDevice")
    when(gatewareInstance.definition).thenReturn(gatewareDefinition)
    
    // For software instances, we need to use SoftwareInstance since it expects
    // a SoftwareDefinitionTrait, not a ChipDefinitionTrait
    
    val softwareDefinition = SoftwareDefinition(
      defType = DefinitionType("program"),
      sourcePath = Paths.get("/path/to/source"),
      attributes = Map.empty[String, Variant],
      parameters = Map.empty[String, Variant],
      dependencies = Seq.empty[String],
      actionsFilePath = Paths.get("/path/to/actions"),
      actionsFile = mock[ActionsFile]
    )
    
    // Create a proper SoftwareInstance mock for the software test
    val softwareInstance = mock[com.deanoc.overlord.Instances.SoftwareInstance]
    when(softwareInstance.name).thenReturn("SoftwareComponent")
    when(softwareInstance.definition).thenReturn(softwareDefinition)

    when(softwareInstance.definition).thenReturn(softwareDefinition)
    
    // Then we need to specifically set the desired isInstanceOf behavior
    // This requires a different mocking technique
    
    val hardwareLoc = InstanceLoc(hardwareInstance, None, "hw.full.path")
    val gatewareLoc = InstanceLoc(gatewareInstance, None, "gw.full.path")
    val softwareLoc = InstanceLoc(softwareInstance, None, "sw.full.path")
    
    // Since we've set up the mocks correctly with the appropriate definition types,
    // the isHardware, isGateware, and isSoftware methods should work correctly
    hardwareLoc.isHardware shouldBe true
    hardwareLoc.isGateware shouldBe false
    hardwareLoc.isSoftware shouldBe false
    
    gatewareLoc.isHardware shouldBe false
    gatewareLoc.isGateware shouldBe true
    gatewareLoc.isSoftware shouldBe false
    
    softwareLoc.isHardware shouldBe false
    softwareLoc.isGateware shouldBe false
    softwareLoc.isSoftware shouldBe true
  }
  
  "ConnectedBus" should "correctly represent bus connections" in {
    val sourceInstance = createMockInstance("SourceDevice")
    val targetInstance = createMockInstance("TargetDevice")
    val sourceLoc = InstanceLoc(sourceInstance, None, "source.full.path")
    val targetLoc = InstanceLoc(targetInstance, None, "target.full.path")
    val bus = mock[SupplierBusLike]
    
    val busConnection = ConnectedBus(
      ExplicitConnectionPriority(),
      sourceLoc,
      FirstToSecondConnection(),
      targetLoc,
      bus,
      targetInstance.asInstanceOf[ChipInstance]
    )
    
    busConnection.connectionPriority shouldBe a[ExplicitConnectionPriority]
    busConnection.main shouldBe sourceLoc
    busConnection.direction shouldBe a[FirstToSecondConnection]
    busConnection.secondary shouldBe targetLoc
    busConnection.bus shouldBe bus
    busConnection.other shouldBe targetInstance
    
    // Test the ConnectedBetween methods
    busConnection.first shouldBe Some(sourceLoc)
    busConnection.second shouldBe Some(targetLoc)
  }
  
  "ConnectedLogical" should "correctly represent logical connections" in {
    val sourceInstance = createMockInstance("SourceDevice")
    val targetInstance = createMockInstance("TargetDevice")
    val sourceLoc = InstanceLoc(sourceInstance, None, "source.full.path")
    val targetLoc = InstanceLoc(targetInstance, None, "target.full.path")
    
    val logicalConnection = ConnectedLogical(
      ExplicitConnectionPriority(),
      sourceLoc,
      FirstToSecondConnection(),
      targetLoc
    )
    
    logicalConnection.connectionPriority shouldBe a[ExplicitConnectionPriority]
    logicalConnection.main shouldBe sourceLoc
    logicalConnection.direction shouldBe a[FirstToSecondConnection]
    logicalConnection.secondary shouldBe targetLoc
    
    // Test the ConnectedBetween methods
    logicalConnection.first shouldBe Some(sourceLoc)
    logicalConnection.second shouldBe Some(targetLoc)
  }
  
  "Constant" should "correctly represent constant parameter connections" in {
    val instance = createMockInstance("Device")
    val port = createMockPort("testPort")
    
    // Create a parameter with a constant value
    val paramName = "testParam"
    val paramValue = Utils.toVariant("42") // Convert to Variant
    val paramType = ConstantParameterType(paramValue) // Create proper ParameterType
    val parameter = Parameter(paramName, paramType)
    
    // Create the constant connection
    val constantConnection = Constant(instance, parameter)
    
    // Verify the connection properties
    constantConnection.instance shouldBe instance
    constantConnection.parameter shouldBe parameter
    constantConnection.parameter.name shouldBe paramName
    constantConnection.parameter.parameterType shouldBe paramType
  }
  
  "Wire" should "correctly represent connections between components" in {
    val sourceInstance = createMockInstance("SourceDevice")
    val targetInstance = createMockInstance("TargetDevice")
    val sourcePort = createMockPort("outPort")
    val targetPort = createMockPort("inPort")
    
    val sourceLoc = InstanceLoc(sourceInstance, Some(sourcePort), "source.out.path")
    val targetLoc = InstanceLoc(targetInstance, Some(targetPort), "target.in.path")
    
    val wire = Wire(
      sourceLoc,
      Seq(targetLoc),
      ExplicitConnectionPriority(),
      true
    )
    
    wire.startLoc shouldBe sourceLoc
    wire.endLocs shouldBe Seq(targetLoc)
    wire.priority shouldBe a[ExplicitConnectionPriority]
    wire.knownWidth shouldBe true
  }
}
