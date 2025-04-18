package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.config._
import io.circe.Json

/** Simple test suite for the ConnectionParser object.
  *
  * This test suite focuses on the public interface of ConnectionParser and
  * demonstrates the use of type-safe configuration classes.
  */
class ConnectionParserBasicSpec
    extends AnyFlatSpec
    with Matchers
    with SilentLogger {

  /** Helper method to create a test Variant for connection testing. This is
    * kept for backward compatibility testing.
    *
    * @param connType
    *   The connection type (e.g., "port", "bus")
    * @param connection
    *   The connection string (e.g., "device1 -> device2")
    * @return
    *   A Variant containing the connection information
    */
  private def createTestVariant(
      connType: String,
      connection: String
  ): Variant = {
    val map = new java.util.HashMap[String, Any]()
    map.put("type", connType)
    map.put("connection", connection)
    Utils.toVariant(map)
  }

  /** Helper method to create a test PortConnectionConfig for connection
    * testing.
    *
    * @param connection
    *   The connection string (e.g., "device1 -> device2")
    * @return
    *   A PortConnectionConfig containing the connection information
    */
  private def createPortConfig(connection: String): PortConnectionConfig = {
    PortConnectionConfig(
      connection = connection,
      `type` = "port"
    )
  }

  /** Helper method to create a test BusConnectionConfig for connection testing.
    *
    * @param connection
    *   The connection string (e.g., "device1 -> device2")
    * @param busProtocol
    *   The bus protocol (e.g., "axi")
    * @param busName
    *   The bus name (optional)
    * @return
    *   A BusConnectionConfig containing the connection information
    */
  private def createBusConfig(
      connection: String,
      busProtocol: String = "internal",
      busName: Option[String] = None
  ): BusConnectionConfig = {
    BusConnectionConfig(
      connection = connection,
      `type` = "bus",
      bus_protocol = Some(busProtocol),
      bus_name = busName
    )
  }

  "ConnectionParser.parseConnectionConfig" should "correctly parse a first-to-second connection" in {
    // Create a type-safe config with the connection string
    val config = createPortConfig("device1 -> device2")
    val result = ConnectionParser.parseConnectionConfig(config)

    // Verify we got a result
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    assert(port.direction == ConnectionDirection.FirstToSecond)
    port.secondFullName shouldBe "device2"
  }

  it should "correctly parse a bidirectional connection" in {
    val config = createPortConfig("device1 <-> device2")
    val result = ConnectionParser.parseConnectionConfig(config)

    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    assert(port.direction == ConnectionDirection.BiDirectional)
    port.secondFullName shouldBe "device2"
  }

  it should "also accept <> as a bidirectional connection" in {
    val config = createPortConfig("device1 <> device2")
    val result = ConnectionParser.parseConnectionConfig(config)

    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    assert(port.direction == ConnectionDirection.BiDirectional)
    port.secondFullName shouldBe "device2"
  }

  it should "correctly parse a second-to-first connection" in {
    val config = createPortConfig("device1 <- device2")
    val result = ConnectionParser.parseConnectionConfig(config)

    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    assert(port.direction == ConnectionDirection.SecondToFirst)
    port.secondFullName shouldBe "device2"
  }

  it should "reject invalid connection formats" in {
    withSilentLogs {
      // Test with invalid operator
      val invalidOperatorConfig = createPortConfig("device1 >> device2")
      ConnectionParser.parseConnectionConfig(
        invalidOperatorConfig
      ) shouldBe None

      // Test with incomplete connection (missing second part)
      val incompleteConfig = createPortConfig("device1 ->")
      ConnectionParser.parseConnectionConfig(incompleteConfig) shouldBe None

      // Test with too many parts
      val tooManyPartsConfig = createPortConfig("device1 -> device2 -> device3")
      ConnectionParser.parseConnectionConfig(tooManyPartsConfig) shouldBe None
    }
  }

  it should "handle different connection types correctly" in {
    // Bus connection
    val busConfig = createBusConfig("device1 -> device2", "axi")
    val busResult = ConnectionParser.parseConnectionConfig(busConfig)

    busResult shouldBe defined
    busResult.get shouldBe a[UnconnectedBus]
    val bus = busResult.get.asInstanceOf[UnconnectedBus]
    bus.busProtocol.value shouldBe "axi"

    // Logical connection
    val logicalConfig = LogicalConnectionConfig(
      connection = "device1 -> device2",
      `type` = "logical"
    )
    val logicalResult = ConnectionParser.parseConnectionConfig(logicalConfig)

    logicalResult shouldBe defined
    logicalResult.get shouldBe a[UnconnectedLogical]

    // Clock connection
    val clockConfig = ClockConnectionConfig(
      connection = "clock -> device",
      `type` = "clock"
    )
    val clockResult = ConnectionParser.parseConnectionConfig(clockConfig)

    clockResult shouldBe defined
    clockResult.get shouldBe a[UnconnectedClock]
  }

  it should "handle port group connections with prefixes and excludes" in {
    val portGroupConfig = PortGroupConnectionConfig(
      connection = "device1 -> device2",
      `type` = "port_group",
      first_prefix = Some("prefix1_"),
      second_prefix = Some("prefix2_"),
      excludes = Some(List("exclude1", "exclude2"))
    )

    val result = ConnectionParser.parseConnectionConfig(portGroupConfig)

    result shouldBe defined
    val portGroup = result.get.asInstanceOf[UnconnectedPortGroup]
    portGroup.firstFullName shouldBe "device1"
    portGroup.secondFullName shouldBe "device2"
    portGroup.first_prefix shouldBe "prefix1_"
    portGroup.second_prefix shouldBe "prefix2_"
    portGroup.excludes should contain allOf ("exclude1", "exclude2")
  }

  it should "handle parameters connections" in {
    val parametersConfig = ParametersConnectionConfig(
      connection = "_ -> device",
      `type` = "parameters",
      parameters = List(
        ParameterConfig(
          name = "param1",
          value = Json.fromString("value1")
        ),
        ParameterConfig(
          name = "param2",
          value = Json.fromInt(42),
          `type` = Some("frequency")
        )
      )
    )

    val result = ConnectionParser.parseConnectionConfig(parametersConfig)

    result shouldBe defined
    result.get shouldBe a[UnconnectedParameters]
    val params = result.get.asInstanceOf[UnconnectedParameters]
    params.instanceName shouldBe "device"
    params.parameters should have size 2
    params.parameters.map(_.name) should contain allOf ("param1", "param2")
  }

  // For backward compatibility, also test the old parseConnection method
  "ConnectionParser.parseConnection" should "still work with Variant-based input" in {
    // Create variant with the connection string
    val variant = createTestVariant("port", "device1 -> device2")
    val result = ConnectionParser.parseConnection(variant)

    // Verify we got a result
    result shouldBe defined
    val port = result.get.asInstanceOf[UnconnectedPortGroup]
    port.firstFullName shouldBe "device1"
    assert(port.direction == ConnectionDirection.FirstToSecond)
    port.secondFullName shouldBe "device2"
  }
}
