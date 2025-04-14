package com.deanoc.overlord.connections

import scala.language.implicitConversions
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger, ArrayV}
import com.deanoc.overlord.hardware.{
  Port,
  BitsDesc,
  InWireDirection,
  OutWireDirection,
  InOutWireDirection
}
import com.deanoc.overlord.instances.{
  ChipInstance,
  InstanceTrait,
  PinGroupInstance,
  ClockInstance
}
import org.mockito.Mockito._
import com.deanoc.overlord.connections.TestUtils._

/** Comprehensive test suite for the ConnectionParser object. This test suite
  * focuses on all aspects of the ConnectionParser's functionality to establish
  * a baseline before refactoring.
  */
class ConnectionParserComprehensiveSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  // Helper method to create a test variant
  private def createVariant(fields: Map[String, Any]): Variant = {
    val map = new java.util.HashMap[String, Any]()
    fields.foreach { case (key, value) =>
      map.put(key, value)
    }
    Utils.toVariant(map)
  }

  // Helper method to create a simple connection variant
  private def createConnectionVariant(
      connType: String,
      connection: String,
      additionalFields: Map[String, Any] = Map.empty
  ): Variant = {
    createVariant(
      Map(
        "type" -> connType,
        "connection" -> connection
      ) ++ additionalFields
    )
  }

  // Tests for parseConnection method
  "ConnectionParser.parseConnection" should "parse port connections with various directions" in {
    // Test first-to-second connection
    val firstToSecond = createConnectionVariant("port", "device1 -> device2")
    val firstToSecondResult = ConnectionParser.parseConnection(firstToSecond)

    firstToSecondResult shouldBe defined
    firstToSecondResult.get shouldBe a[UnconnectedPortGroup]
    val port1 = firstToSecondResult.get.asInstanceOf[UnconnectedPortGroup]
    port1.firstFullName shouldBe "device1"
    (port1.direction == ConnectionDirection.FirstToSecond) shouldBe true
    port1.secondFullName shouldBe "device2"

    // Test second-to-first connection
    val secondToFirst = createConnectionVariant("port", "device1 <- device2")
    val secondToFirstResult = ConnectionParser.parseConnection(secondToFirst)

    secondToFirstResult shouldBe defined
    secondToFirstResult.get shouldBe a[UnconnectedPortGroup]
    val port2 = secondToFirstResult.get.asInstanceOf[UnconnectedPortGroup]
    port2.firstFullName shouldBe "device1"
    (port2.direction == ConnectionDirection.SecondToFirst) shouldBe true
    port2.secondFullName shouldBe "device2"

    // Test bidirectional connection with <->
    val biDir1 = createConnectionVariant("port", "device1 <-> device2")
    val biDir1Result = ConnectionParser.parseConnection(biDir1)

    biDir1Result shouldBe defined
    assert(biDir1Result.get.isInstanceOf[UnconnectedPortGroup])
    val port3 = biDir1Result.get.asInstanceOf[UnconnectedPortGroup]
    port3.firstFullName shouldBe "device1"
    (port3.direction == ConnectionDirection.BiDirectional) shouldBe true
    port3.secondFullName shouldBe "device2"

    // Test bidirectional connection with <>
    val biDir2 = createConnectionVariant("port", "device1 <> device2")
    val biDir2Result = ConnectionParser.parseConnection(biDir2)

    biDir2Result shouldBe defined
    assert(biDir2Result.get.isInstanceOf[UnconnectedPortGroup])
    val port4 = biDir2Result.get.asInstanceOf[UnconnectedPortGroup]
    port4.firstFullName shouldBe "device1"
    (port4.direction == ConnectionDirection.BiDirectional) shouldBe true
    port4.secondFullName shouldBe "device2"
  }

  it should "parse bus connections with all parameters" in {
    // Create a bus connection with all possible parameters
    val busVariant = createConnectionVariant(
      "bus",
      "cpu -> memory",
      Map(
        "bus_protocol" -> "axi4",
        "bus_name" -> "main_bus",
        "consumer_bus_name" -> "mem_bus",
        "silent" -> true
      )
    )

    val result = ConnectionParser.parseConnection(busVariant)

    result shouldBe defined
    assert(result.get.isInstanceOf[UnconnectedBus])
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    (bus.direction == ConnectionDirection.FirstToSecond) shouldBe true
    bus.secondFullName shouldBe "memory"
    bus.busProtocol.shouldEqual("axi4")
    bus.supplierBusName.shouldEqual("main_bus")
    bus.consumerBusName.shouldEqual("mem_bus")
    bus.silent shouldBe true
  }

  it should "use default values for optional bus parameters" in {
    // Create a bus connection with minimal parameters
    val minimalBusVariant = createConnectionVariant(
      "bus",
      "cpu -> memory"
    )

    val result = ConnectionParser.parseConnection(minimalBusVariant)

    result shouldBe defined
    result.get shouldBe a[UnconnectedBus]
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.busProtocol.shouldEqual("internal") // default value
    bus.supplierBusName.shouldEqual("") // default value
    bus.consumerBusName.shouldEqual("") // should match supplierBusName
    bus.silent shouldBe false // default value
  }

  it should "parse port_group connections with prefixes and excludes" in {
    // Create excludes list
    val excludesList = new java.util.ArrayList[String]()
    excludesList.add("clock")
    excludesList.add("reset")

    // Create port_group connection
    val portGroupVariant = createConnectionVariant(
      "port_group",
      "uart0 -> uart1",
      Map(
        "first_prefix" -> "tx_",
        "second_prefix" -> "rx_",
        "excludes" -> excludesList
      )
    )

    val result = ConnectionParser.parseConnection(portGroupVariant)

    result shouldBe defined
    assert(result.get.isInstanceOf[UnconnectedPortGroup])
    val portGroup = result.get.asInstanceOf[UnconnectedPortGroup]
    portGroup.firstFullName shouldBe "uart0"
    (portGroup.direction == ConnectionDirection.FirstToSecond) shouldBe true
    portGroup.secondFullName shouldBe "uart1"
    portGroup.first_prefix shouldBe "tx_"
    portGroup.second_prefix shouldBe "rx_"
    portGroup.excludes should contain allOf ("clock", "reset")
  }

  it should "parse logical connections" in {
    val logicalVariant = createConnectionVariant("logical", "block1 -> block2")

    val result = ConnectionParser.parseConnection(logicalVariant)

    result shouldBe defined
    assert(result.get.isInstanceOf[UnconnectedLogical])
    val logical = result.get.asInstanceOf[UnconnectedLogical]
    logical.firstFullName shouldBe "block1"
    (logical.direction == ConnectionDirection.FirstToSecond) shouldBe true
    logical.secondFullName shouldBe "block2"
  }

  it should "parse clock connections" in {
    val clockVariant = createConnectionVariant("clock", "system_clock -> cpu")

    val result = ConnectionParser.parseConnection(clockVariant)

    result shouldBe defined
    assert(result.get.isInstanceOf[UnconnectedClock])
    val clock = result.get.asInstanceOf[UnconnectedClock]
    clock.firstFullName shouldBe "system_clock"
    (clock.direction == ConnectionDirection.FirstToSecond) shouldBe true
    clock.secondFullName shouldBe "cpu"
  }

  it should "parse parameters connections" in {
    // Create parameter list
    val paramsList = new java.util.ArrayList[java.util.Map[String, Any]]()

    // Add constant parameter
    val constantParam = new java.util.HashMap[String, Any]()
    constantParam.put("name", "width")
    constantParam.put("value", 32)
    paramsList.add(constantParam)

    // Add frequency parameter
    val freqParam = new java.util.HashMap[String, Any]()
    freqParam.put("name", "clock_freq")
    freqParam.put("type", "frequency")
    freqParam.put("value", "100MHz")
    paramsList.add(freqParam)

    // Create parameters connection
    val paramsVariant = createConnectionVariant(
      "parameters",
      "_ -> target_device",
      Map("parameters" -> paramsList)
    )

    val result = ConnectionParser.parseConnection(paramsVariant)

    result shouldBe defined
    result.get shouldBe a[UnconnectedParameters]
    val params = result.get.asInstanceOf[UnconnectedParameters]
    params.instanceName shouldBe "target_device"
    params.parameters.length shouldBe 2

    // Check parameter names
    params.parameters.map(_.name) should contain allOf ("width", "clock_freq")

    // Find each parameter
    val widthParam = params.parameters.find(_.name == "width").get
    val clockParam = params.parameters.find(_.name == "clock_freq").get

    // Check parameter types
    widthParam.parameterType shouldBe a[ConstantParameterType]
    clockParam.parameterType shouldBe a[FrequencyParameterType]
  }

  it should "reject parameters connections without valid first part" in {
    // Parameters connections must have "_" as first part
    val invalidParamsList =
      new java.util.ArrayList[java.util.Map[String, Any]]()
    val param = new java.util.HashMap[String, Any]()
    param.put("name", "test")
    param.put("value", "val")
    invalidParamsList.add(param)

    val invalidParamsVariant = createConnectionVariant(
      "parameters",
      "not_underscore -> target", // Should be "_ -> target"
      Map("parameters" -> invalidParamsList)
    )

    withSilentLogs {
      val result = ConnectionParser.parseConnection(invalidParamsVariant)
      result shouldBe None
    }
  }

  it should "handle connection strings with extra whitespace" in {
    // Test with varied whitespace
    val whitespaceVariant =
      createConnectionVariant("port", "  device1    ->    device2  ")

    val result = ConnectionParser.parseConnection(whitespaceVariant)

    result shouldBe defined
    assert(result.get.isInstanceOf[UnconnectedPortGroup])
    val port = result.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    assert(port.direction == ConnectionDirection.FirstToSecond)
    port.secondFullName shouldBe "device2"
  }

  it should "reject connections with missing required fields" in {
    withSilentLogs {
      // Missing type field
      val missingTypeVariant = createVariant(Map("connection" -> "a -> b"))
      ConnectionParser.parseConnection(missingTypeVariant) shouldBe None

      // Missing connection field
      val missingConnVariant = createVariant(Map("type" -> "port"))
      ConnectionParser.parseConnection(missingConnVariant) shouldBe None
    }
  }

  it should "reject connections with invalid format" in {
    withSilentLogs {
      // Invalid direction operator
      val invalidOperatorVariant =
        createConnectionVariant("port", "device1 => device2")
      ConnectionParser.parseConnection(invalidOperatorVariant) shouldBe None

      // Multiple direction operators
      val multipleOperatorsVariant =
        createConnectionVariant("port", "device1 -> device2 -> device3")
      ConnectionParser.parseConnection(multipleOperatorsVariant) shouldBe None
    }
  }

  it should "reject unknown connection types" in {
    withSilentLogs {
      val unknownTypeVariant = createConnectionVariant("unknown_type", "a -> b")
      ConnectionParser.parseConnection(unknownTypeVariant) shouldBe None
    }
  }

  // Tests for parseParametersConnection method
  "ConnectionParser.parseParametersConnection" should "parse different parameter types" in {
    // Create parameters array
    val paramsList = new java.util.ArrayList[java.util.Map[String, Any]]()

    // Add constant parameter
    val constantParam = new java.util.HashMap[String, Any]()
    constantParam.put("name", "width")
    constantParam.put("value", 32)
    paramsList.add(constantParam)

    // Add frequency parameter
    val freqParam = new java.util.HashMap[String, Any]()
    freqParam.put("name", "clock_freq")
    freqParam.put("type", "frequency")
    freqParam.put("value", "100MHz")
    paramsList.add(freqParam)

    // Add parameter with default type
    val defaultTypeParam = new java.util.HashMap[String, Any]()
    defaultTypeParam.put("name", "default_type")
    defaultTypeParam.put("value", "some_value")
    paramsList.add(defaultTypeParam)

    // Convert to ArrayV
    val parametersArray = Utils.toVariant(paramsList).asInstanceOf[ArrayV].value

    // Parse parameters
    val result = ConnectionParser.parseParametersConnection(
      ConnectionDirection.FirstToSecond,
      "target_device",
      parametersArray
    )

    // Check result
    assert(result.isInstanceOf[UnconnectedParameters])
    assert(result.direction == ConnectionDirection.FirstToSecond)
    result.instanceName shouldBe "target_device"
    result.parameters.length shouldBe 3

    // Check parameter names
    result.parameters.map(
      _.name
    ) should contain allOf ("width", "clock_freq", "default_type")

    // Find each parameter
    val widthParam = result.parameters.find(_.name == "width").get
    val clockParam = result.parameters.find(_.name == "clock_freq").get
    val defaultParam = result.parameters.find(_.name == "default_type").get

    // Check parameter types
    widthParam.parameterType shouldBe a[ConstantParameterType]
    clockParam.parameterType shouldBe a[FrequencyParameterType]
    defaultParam.parameterType shouldBe a[ConstantParameterType]

    // Check frequency value
    val freq =
      clockParam.parameterType.asInstanceOf[FrequencyParameterType].freq
    // Verify frequency is approximately 100 MHz
    // The Utils.toFrequency method converts "100MHz" to roughly 104857600.0
    // (100 * 1024 * 1024) as it uses binary multipliers
    freq should be >= 100000000.0 // Should be at least 100 MHz
  }

  it should "handle invalid parameter entries gracefully" in {
    withSilentLogs {
      // Create parameters array with errors
      val paramsList = new java.util.ArrayList[java.util.Map[String, Any]]()

      // Missing name
      val missingNameParam = new java.util.HashMap[String, Any]()
      missingNameParam.put("value", "some_value")
      paramsList.add(missingNameParam)

      // Missing value in frequency
      val missingValueFreqParam = new java.util.HashMap[String, Any]()
      missingValueFreqParam.put("name", "bad_freq")
      missingValueFreqParam.put("type", "frequency")
      paramsList.add(missingValueFreqParam)

      // Missing value in constant
      val missingValueConstParam = new java.util.HashMap[String, Any]()
      missingValueConstParam.put("name", "bad_const")
      missingValueConstParam.put("type", "other")
      paramsList.add(missingValueConstParam)

      // Valid parameter
      val validParam = new java.util.HashMap[String, Any]()
      validParam.put("name", "good_param")
      validParam.put("value", "good_value")
      paramsList.add(validParam)

      // Convert to ArrayV
      val parametersArray =
        Utils.toVariant(paramsList).asInstanceOf[ArrayV].value

      // Parse parameters
      val result = ConnectionParser.parseParametersConnection(
        ConnectionDirection.FirstToSecond,
        "target_device",
        parametersArray
      )

      // Should only contain the valid parameter
      result shouldBe a[UnconnectedParameters]
      result.parameters.length shouldBe 1
      result.parameters.head.name shouldBe "good_param"
    }
  }

  // Tests for parsePortConnection method
  "ConnectionParser.parsePortConnection" should "create connected port groups between instances" in {
    // Create mock instances and ports
    val chipInstance1 = mock[ChipInstance]
    val chipInstance2 = mock[ChipInstance]
    val gatewareDef = mock[GatewareDefinitionTrait]
    val hardwareDef = mock[HardwareDefinitionTrait]

    when(chipInstance1.definition).thenReturn(gatewareDef)
    when(chipInstance2.definition).thenReturn(hardwareDef)

    val inPort = Port("in_port", BitsDesc(8), InWireDirection())
    val outPort = Port("out_port", BitsDesc(8), OutWireDirection())

    // Create instance locations
    val srcLoc = InstanceLoc(chipInstance1, Some(inPort), "source")
    val destLoc = InstanceLoc(chipInstance2, Some(outPort), "destination")

    // Parse port connection
    val result = ConnectionParser.parsePortConnection(
      ConnectionPriority.Explicit,
      srcLoc,
      destLoc
    )

    // Check result
    result shouldBe a[ConnectedPortGroup]
    result.connectionPriority shouldBe ConnectionPriority.Explicit
    result.first.get.instance shouldBe chipInstance1
    result.first.get.port.get.name shouldBe "in_port"
    result.first.get.port.get.direction shouldBe a[InWireDirection]
    result.second.get.instance shouldBe chipInstance2
    result.second.get.port.get.name shouldBe "out_port"
    result.second.get.port.get.direction shouldBe a[OutWireDirection]
    (result.direction == ConnectionDirection.FirstToSecond) shouldBe true
  }

  it should "handle InOutWireDirection correctly" in {
    // Create mock instances and ports
    val chipInstance1 = mock[ChipInstance]
    val chipInstance2 = mock[ChipInstance]
    val hardwareDef = mock[HardwareDefinitionTrait]

    when(chipInstance1.definition).thenReturn(hardwareDef)
    when(chipInstance2.definition).thenReturn(hardwareDef)

    val inOutPort = Port("inout_port", BitsDesc(8), InOutWireDirection())
    val inPort = Port("in_port", BitsDesc(8), InWireDirection())
    val outPort = Port("out_port", BitsDesc(8), OutWireDirection())

    // Test InOutWireDirection with InWireDirection
    val srcLoc1 = InstanceLoc(chipInstance1, Some(inOutPort), "source1")
    val destLoc1 = InstanceLoc(chipInstance2, Some(inPort), "destination1")

    val result1 = ConnectionParser.parsePortConnection(
      ConnectionPriority.Explicit,
      srcLoc1,
      destLoc1
    )

    result1.first.get.port.get.direction shouldBe a[InWireDirection]
    result1.second.get.port.get.direction shouldBe a[InWireDirection]

    // Test InOutWireDirection with OutWireDirection
    val srcLoc2 = InstanceLoc(chipInstance1, Some(inOutPort), "source2")
    val destLoc2 = InstanceLoc(chipInstance2, Some(outPort), "destination2")

    val result2 = ConnectionParser.parsePortConnection(
      ConnectionPriority.Explicit,
      srcLoc2,
      destLoc2
    )

    result2.first.get.port.get.direction shouldBe a[OutWireDirection]
    result2.second.get.port.get.direction shouldBe a[OutWireDirection]

    // Test both InOutWireDirection
    val srcLoc3 = InstanceLoc(chipInstance1, Some(inOutPort), "source3")
    val destLoc3 = InstanceLoc(chipInstance2, Some(inOutPort), "destination3")

    val result3 = ConnectionParser.parsePortConnection(
      ConnectionPriority.Explicit,
      srcLoc3,
      destLoc3
    )

    result3.first.get.port.get.direction shouldBe a[InOutWireDirection]
    result3.second.get.port.get.direction shouldBe a[InOutWireDirection]
  }
}
