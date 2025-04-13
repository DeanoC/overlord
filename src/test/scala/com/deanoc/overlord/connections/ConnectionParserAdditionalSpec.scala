package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.connections.ConnectionTypes.BusName
import scala.language.implicitConversions
import com.deanoc.overlord.connections.TestUtils._

/** Additional test suite for the ConnectionParser focusing on edge cases and
  * error handling. This test suite extends the existing test coverage for the
  * ConnectionParser.
  */
class ConnectionParserAdditionalSpec
    extends AnyFlatSpec
    with Matchers
    with SilentLogger {

  /** Helper method to create a test Variant for connection testing.
    */
  private def createTestVariant(fields: Map[String, Any]): Variant = {
    val map = new java.util.HashMap[String, Any]()
    fields.foreach { case (key, value) =>
      map.put(key, value)
    }
    Utils.toVariant(map)
  }

  "ConnectionParser.parseConnection" should "handle malformed connection strings gracefully" in {
    withSilentLogs {
      // Test with empty connection string
      val emptyConnectionVariant = createTestVariant(
        Map(
          "type" -> "port",
          "connection" -> ""
        )
      )
      ConnectionParser.parseConnection(emptyConnectionVariant) shouldBe None

      // Test with connection string containing only spaces
      val spacesOnlyVariant = createTestVariant(
        Map(
          "type" -> "port",
          "connection" -> "   "
        )
      )
      ConnectionParser.parseConnection(spacesOnlyVariant) shouldBe None

      // Test with connection string containing only the direction operator
      val operatorOnlyVariant = createTestVariant(
        Map(
          "type" -> "port",
          "connection" -> "->"
        )
      )
      ConnectionParser.parseConnection(operatorOnlyVariant) shouldBe None
    }
  }

  it should "handle edge cases in connection string parsing" in {
    withSilentLogs {
      // Test with connection string containing special characters
      val specialCharsVariant = createTestVariant(
        Map(
          "type" -> "port",
          "connection" -> "device@123 -> device#456"
        )
      )
      val specialCharsResult =
        ConnectionParser.parseConnection(specialCharsVariant)
      specialCharsResult shouldBe defined
      val specialCharsPort =
        specialCharsResult.get.asInstanceOf[UnconnectedPort]
      specialCharsPort.firstFullName shouldBe "device@123"
      specialCharsPort.secondFullName shouldBe "device#456"

      // Test with connection string containing path-like names
      val pathLikeVariant = createTestVariant(
        Map(
          "type" -> "port",
          "connection" -> "/path/to/device1 -> /path/to/device2"
        )
      )
      val pathLikeResult = ConnectionParser.parseConnection(pathLikeVariant)
      pathLikeResult shouldBe defined
      val pathLikePort = pathLikeResult.get.asInstanceOf[UnconnectedPort]
      pathLikePort.firstFullName shouldBe "/path/to/device1"
      pathLikePort.secondFullName shouldBe "/path/to/device2"
    }
  }

  it should "handle bus connections with complex parameters" in {
    // Test with bus connection having multiple parameters
    val complexBusVariant = createTestVariant(
      Map(
        "type" -> "bus",
        "connection" -> "cpu -> memory",
        "bus_protocol" -> "axi4",
        "bus_name" -> "main_bus",
        "consumer_bus_name" -> "mem_bus",
        "silent" -> true,
        "extra_param" -> "should be ignored" // Extra parameter that should be ignored
      )
    )

    val result = ConnectionParser.parseConnection(complexBusVariant)

    result shouldBe defined
    result.get shouldBe a[UnconnectedBus]
    val bus = result.get.asInstanceOf[UnconnectedBus]
    bus.firstFullName shouldBe "cpu"
    bus.secondFullName shouldBe "memory"
    bus.busProtocol.shouldEqual("axi4")
    bus.supplierBusName.shouldEqual("main_bus")
    bus.consumerBusName.shouldEqual("mem_bus")
    bus.silent shouldBe true
  }

  it should "handle clock connections with different formats" in {
    // Test with standard clock connection
    val standardClockVariant = createTestVariant(
      Map(
        "type" -> "clock",
        "connection" -> "system_clock -> cpu"
      )
    )

    val standardResult = ConnectionParser.parseConnection(standardClockVariant)
    standardResult shouldBe defined
    standardResult.get shouldBe a[UnconnectedClock]

    // Test with clock connection having frequency parameter
    val freqClockVariant = createTestVariant(
      Map(
        "type" -> "clock",
        "connection" -> "system_clock -> cpu",
        "frequency" -> "100MHz"
      )
    )

    val freqResult = ConnectionParser.parseConnection(freqClockVariant)
    freqResult shouldBe defined
    freqResult.get shouldBe a[UnconnectedClock]

    // Test with bidirectional clock connection (unusual but should be handled)
    val biDirClockVariant = createTestVariant(
      Map(
        "type" -> "clock",
        "connection" -> "system_clock <-> cpu"
      )
    )

    val biDirResult = ConnectionParser.parseConnection(biDirClockVariant)
    biDirResult shouldBe defined
    biDirResult.get shouldBe a[UnconnectedClock]
    val biDirClock = biDirResult.get.asInstanceOf[UnconnectedClock]
    biDirClock.direction shouldBe ConnectionDirection.BiDirectional
  }

  it should "handle logical connections with different directions" in {
    // Test with first to second logical connection
    val firstToSecondVariant = createTestVariant(
      Map(
        "type" -> "logical",
        "connection" -> "block1 -> block2"
      )
    )

    val firstToSecondResult =
      ConnectionParser.parseConnection(firstToSecondVariant)
    firstToSecondResult shouldBe defined
    firstToSecondResult.get shouldBe a[UnconnectedLogical]
    val firstToSecondLogical =
      firstToSecondResult.get.asInstanceOf[UnconnectedLogical]
    firstToSecondLogical.direction shouldBe ConnectionDirection.FirstToSecond

    // Test with second to first logical connection
    val secondToFirstVariant = createTestVariant(
      Map(
        "type" -> "logical",
        "connection" -> "block1 <- block2"
      )
    )

    val secondToFirstResult =
      ConnectionParser.parseConnection(secondToFirstVariant)
    secondToFirstResult shouldBe defined
    secondToFirstResult.get shouldBe a[UnconnectedLogical]
    val secondToFirstLogical =
      secondToFirstResult.get.asInstanceOf[UnconnectedLogical]
    secondToFirstLogical.direction shouldBe ConnectionDirection.SecondToFirst

    // Test with bidirectional logical connection
    val biDirVariant = createTestVariant(
      Map(
        "type" -> "logical",
        "connection" -> "block1 <-> block2"
      )
    )

    val biDirResult = ConnectionParser.parseConnection(biDirVariant)
    biDirResult shouldBe defined
    biDirResult.get shouldBe a[UnconnectedLogical]
    val biDirLogical = biDirResult.get.asInstanceOf[UnconnectedLogical]
    biDirLogical.direction shouldBe ConnectionDirection.BiDirectional
  }

  it should "handle port_group connections with various prefixes and excludes" in {
    // Create excludes list
    val excludesList = new java.util.ArrayList[String]()
    excludesList.add("clock")
    excludesList.add("reset")

    // Test with standard port_group connection
    val standardPortGroupVariant = createTestVariant(
      Map(
        "type" -> "port_group",
        "connection" -> "uart0 -> uart1",
        "first_prefix" -> "tx_",
        "second_prefix" -> "rx_",
        "excludes" -> excludesList
      )
    )

    val result = ConnectionParser.parseConnection(standardPortGroupVariant)

    result shouldBe defined
    result.get shouldBe a[UnconnectedPortGroup]
    val portGroup = result.get.asInstanceOf[UnconnectedPortGroup]
    portGroup.firstFullName shouldBe "uart0"
    portGroup.secondFullName shouldBe "uart1"
    portGroup.first_prefix shouldBe "tx_"
    portGroup.second_prefix shouldBe "rx_"
    portGroup.excludes should contain allOf ("clock", "reset")

    // Test with empty prefixes
    val emptyPrefixesVariant = createTestVariant(
      Map(
        "type" -> "port_group",
        "connection" -> "uart0 -> uart1"
      )
    )

    val emptyPrefixesResult =
      ConnectionParser.parseConnection(emptyPrefixesVariant)
    emptyPrefixesResult shouldBe defined
    emptyPrefixesResult.get shouldBe a[UnconnectedPortGroup]
    val emptyPrefixesPortGroup =
      emptyPrefixesResult.get.asInstanceOf[UnconnectedPortGroup]
    emptyPrefixesPortGroup.first_prefix shouldBe ""
    emptyPrefixesPortGroup.second_prefix shouldBe ""
    emptyPrefixesPortGroup.excludes shouldBe empty
  }

  it should "handle parameters connections with different parameter types" in {
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

    // Add string parameter
    val stringParam = new java.util.HashMap[String, Any]()
    stringParam.put("name", "description")
    stringParam.put("value", "Test parameter")
    paramsList.add(stringParam)

    // Create parameters connection
    val paramsVariant = createTestVariant(
      Map(
        "type" -> "parameters",
        "connection" -> "_ -> target_device",
        "parameters" -> paramsList
      )
    )

    val result = ConnectionParser.parseConnection(paramsVariant)

    result shouldBe defined
    result.get shouldBe a[UnconnectedParameters]
    val params = result.get.asInstanceOf[UnconnectedParameters]
    params.instanceName shouldBe "target_device"
    params.parameters.length shouldBe 3

    // Check parameter names
    params.parameters.map(
      _.name
    ) should contain allOf ("width", "clock_freq", "description")

    // Find each parameter
    val widthParam = params.parameters.find(_.name == "width").get
    val clockParam = params.parameters.find(_.name == "clock_freq").get
    val descParam = params.parameters.find(_.name == "description").get

    // Check parameter types
    widthParam.parameterType shouldBe a[ConstantParameterType]
    clockParam.parameterType shouldBe a[FrequencyParameterType]
    descParam.parameterType shouldBe a[ConstantParameterType]
  }
}
