package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import scala.language.implicitConversions
import com.deanoc.overlord.connections.ConnectionTypesTestExtensions._

import com.deanoc.overlord._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.interfaces._
import com.deanoc.overlord.connections.ConnectionTypes._
import com.deanoc.overlord.hardware.{
  Port,
  BitsDesc,
  InWireDirection,
  OutWireDirection
}

// Helper methods for testing
object UnconnectedBusAdditionalTestHelpers {
  implicit class BusNameTestOps(name: BusName) {
    def value: String = name.toString
  }
}

/** Additional test suite for UnconnectedBus class focusing on:
  *   1. Testing the `preConnect()` method for error handling 2. Testing the
  *      `connect()` method for different connection scenarios
  */
class UnconnectedBusAdditionalSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  // Test preConnect() method for error handling
  "UnconnectedBus.preConnect" should "handle missing instances gracefully" in {
    // Create mock instance
    val instance = mock[ChipInstance]
    doReturn("device1").when(instance).name

    // Setup getMatchNameAndPort method
    doReturn((Some("device1"), None))
      .when(instance)
      .getMatchNameAndPort("device1")
    doReturn((None, None)).when(instance).getMatchNameAndPort("non_existent")

    // Create UnconnectedBus with non-existent second instance
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "non_existent",
      busProtocol = "axi4"
    )

    // Test preConnect with missing instance
    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq(instance))
      }
    }
  }

  it should "handle bidirectional connections correctly" in {
    // Create mock instances
    val instance1 = mock[ChipInstance]
    val instance2 = mock[ChipInstance]

    doReturn("device1").when(instance1).name
    doReturn("device2").when(instance2).name

    // Setup getMatchNameAndPort method
    doReturn((Some("device1"), None))
      .when(instance1)
      .getMatchNameAndPort("device1")
    doReturn((Some("device2"), None))
      .when(instance2)
      .getMatchNameAndPort("device2")

    // Create UnconnectedBus with bidirectional connection
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "device2",
      busProtocol = "axi4"
    )

    // Test preConnect with bidirectional connection
    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq(instance1, instance2))
      }
    }
  }

  it should "handle instances without required interfaces" in {
    // Create mock instances
    val instance1 = mock[ChipInstance]
    val instance2 = mock[ChipInstance]

    doReturn("device1").when(instance1).name
    doReturn("device2").when(instance2).name

    // Setup getMatchNameAndPort method
    doReturn((Some("device1"), None))
      .when(instance1)
      .getMatchNameAndPort("device1")
    doReturn((Some("device2"), None))
      .when(instance2)
      .getMatchNameAndPort("device2")

    // Setup interfaces
    doReturn(false).when(instance1).hasInterface[PortsLike]
    doReturn(false).when(instance1).hasInterface[MultiBusLike]

    doReturn(false).when(instance2).hasInterface[PortsLike]
    doReturn(false).when(instance2).hasInterface[MultiBusLike]

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2",
      busProtocol = "axi4"
    )

    // Test preConnect with instances missing required interfaces
    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq(instance1, instance2))
      }
    }
  }

  // Test connect() method for different connection scenarios
  "UnconnectedBus.connect" should "create correct connections for hardware instances" in {
    // Create mock instances and interfaces
    val supplierInstance = mock[ChipInstance]
    val consumerInstance = mock[ChipInstance]
    val multiBusInterface = mock[MultiBusLike]
    val supplierBus = mock[SupplierBusLike]
    val consumerBus = mock[BusLike]

    // Setup supplier instance
    doReturn("fpga").when(supplierInstance).name
    doReturn((Some("fpga"), None))
      .when(supplierInstance)
      .getMatchNameAndPort("fpga")
    doReturn(true).when(supplierInstance).hasInterface[MultiBusLike]
    doReturn(Some(multiBusInterface))
      .when(supplierInstance)
      .getInterface[MultiBusLike]

    // Setup consumer instance
    doReturn("ddr").when(consumerInstance).name
    doReturn((Some("ddr"), None))
      .when(consumerInstance)
      .getMatchNameAndPort("ddr")
    doReturn(true).when(consumerInstance).hasInterface[MultiBusLike]
    doReturn(Some(multiBusInterface))
      .when(consumerInstance)
      .getInterface[MultiBusLike]

    // Setup bus interfaces
    doReturn(1).when(multiBusInterface).numberOfBuses
    doReturn(Some(supplierBus))
      .when(multiBusInterface)
      .getFirstSupplierBusByName(anyString())
    doReturn(Some(supplierBus))
      .when(multiBusInterface)
      .getFirstSupplierBusOfProtocol(anyString())
    doReturn(Some(consumerBus))
      .when(multiBusInterface)
      .getFirstConsumerBusByName(anyString())
    doReturn(Some(consumerBus))
      .when(multiBusInterface)
      .getFirstConsumerBusOfProtocol(anyString())

    // Setup supplier bus
    doReturn(true).when(supplierBus).isSupplier
    doReturn(true).when(supplierBus).isHardware
    doReturn("bus_").when(supplierBus).getPrefix

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "fpga",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "ddr",
      busProtocol = "axi4"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result should not be empty
    result.head shouldBe a[ConnectedBus]
    val connectedBus = result.head.asInstanceOf[ConnectedBus]
    connectedBus.firstFullName shouldBe "fpga"
    connectedBus.secondFullName shouldBe "ddr"

    // For hardware instances, we should only have the bus connection, not port connections
    result.length shouldBe 1
  }

  it should "handle silent mode correctly" in {
    // Create mock instance
    val instance = mock[ChipInstance]
    doReturn("cpu").when(instance).name
    doReturn((Some("cpu"), None)).when(instance).getMatchNameAndPort("cpu")
    doReturn((None, None)).when(instance).getMatchNameAndPort("non_existent")

    // Create UnconnectedBus with silent mode
    val silentBus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "non_existent",
      busProtocol = "axi4",
      silent = true
    )

    // Test the connection with silent mode
    withSilentLogs {
      val result = silentBus.connect(Seq(instance))
      result shouldBe empty
    }
  }

  it should "handle SecondToFirst direction correctly" in {
    // Create mock instances and interfaces
    val supplierInstance = mock[ChipInstance]
    val consumerInstance = mock[ChipInstance]
    val multiBusInterface = mock[MultiBusLike]
    val supplierBus = mock[SupplierBusLike]
    val consumerBus = mock[BusLike]

    // Setup supplier instance
    doReturn("cpu").when(supplierInstance).name
    doReturn((Some("cpu"), None))
      .when(supplierInstance)
      .getMatchNameAndPort("cpu")
    doReturn(true).when(supplierInstance).hasInterface[MultiBusLike]
    doReturn(Some(multiBusInterface))
      .when(supplierInstance)
      .getInterface[MultiBusLike]

    // Setup consumer instance
    doReturn("memory").when(consumerInstance).name
    doReturn((Some("memory"), None))
      .when(consumerInstance)
      .getMatchNameAndPort("memory")
    doReturn(true).when(consumerInstance).hasInterface[MultiBusLike]
    doReturn(Some(multiBusInterface))
      .when(consumerInstance)
      .getInterface[MultiBusLike]

    // Setup bus interfaces
    doReturn(1).when(multiBusInterface).numberOfBuses
    doReturn(Some(supplierBus))
      .when(multiBusInterface)
      .getFirstSupplierBusByName(anyString())
    doReturn(Some(supplierBus))
      .when(multiBusInterface)
      .getFirstSupplierBusOfProtocol(anyString())
    doReturn(Some(consumerBus))
      .when(multiBusInterface)
      .getFirstConsumerBusByName(anyString())
    doReturn(Some(consumerBus))
      .when(multiBusInterface)
      .getFirstConsumerBusOfProtocol(anyString())

    // Setup supplier bus
    doReturn(true).when(supplierBus).isSupplier
    doReturn(false).when(supplierBus).isHardware
    doReturn("bus_").when(supplierBus).getPrefix

    // Create UnconnectedBus with SecondToFirst direction
    val bus = UnconnectedBus(
      firstFullName = "memory",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "cpu",
      busProtocol = "axi4"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result should not be empty
    result.head shouldBe a[ConnectedBus]
    val connectedBus = result.head.asInstanceOf[ConnectedBus]
    connectedBus.firstFullName shouldBe "memory"
    connectedBus.secondFullName shouldBe "cpu"
    connectedBus.direction shouldBe ConnectionDirection.SecondToFirst
  }
}
