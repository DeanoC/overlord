package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import com.deanoc.overlord._
import scala.language.implicitConversions
import com.deanoc.overlord.connections.ConnectionTypesTestExtensions._
import scala.language.implicitConversions
import com.deanoc.overlord.connections.ConnectionTypesTestExtensions._
import com.deanoc.overlord.utils.SilentLogger
import com.deanoc.overlord.connections.ConnectionTypes._

// Helper methods for testing
object UnconnectedBusExtendedTestHelpers {
  implicit class BusNameTestOps(name: BusName) {
    def value: String = name.toString
  }
}
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.connections.InstanceLoc
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.interfaces._
import com.deanoc.overlord.hardware.{
  Port,
  BitsDesc,
  InWireDirection,
  OutWireDirection
}
import com.deanoc.overlord.utils.{Variant, BigIntV}

import scala.collection.mutable

/** Extended test suite for UnconnectedBus class focusing on:
  *   1. Testing the `getBus()` method with various input combinations 2.
  *      Testing the `preConnect()` method for error handling 3. Testing the
  *      `connect()` method for different connection scenarios
  */
class UnconnectedBusExtendedSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  // Helper method to create mock instances with interfaces
  private def createMockChipInstance(
      name: String,
      hasPortsInterface: Boolean = true,
      hasMultiBusInterface: Boolean = true,
      isSupplier: Boolean = true,
      busProtocol: String = "default",
      busName: String = "main_bus",
      isHardware: Boolean = false
  ): ChipInstance = {
    val instance = mock[ChipInstance]
    val definition = mock[ChipDefinitionTrait]

    // Setup basic instance properties using doReturn...when pattern
    doReturn(definition).when(instance).definition
    doReturn(name).when(instance).name

    // Setup getMatchNameAndPort method
    doReturn((Some(name), None)).when(instance).getMatchNameAndPort(name)
    doReturn((Some(name), None)).when(instance).getMatchNameAndPort(anyString())

    // Setup PortsLike interface
    if (hasPortsInterface) {
      val portsInterface = mock[PortsLike]
      doReturn(true).when(instance).hasInterface[PortsLike]
      doReturn(portsInterface).when(instance).getInterfaceUnwrapped[PortsLike]

      // Setup port retrieval methods
      val outPorts = Seq(
        Port("bus_data", BitsDesc(32), OutWireDirection()),
        Port("bus_addr", BitsDesc(32), OutWireDirection())
      )
      val inPorts = Seq(
        Port("bus_data", BitsDesc(32), InWireDirection()),
        Port("bus_addr", BitsDesc(32), InWireDirection())
      )

      doReturn(outPorts).when(portsInterface).getPortsStartingWith(anyString())
      doReturn(inPorts).when(portsInterface).getPortsMatchingName(anyString())
    } else {
      doReturn(false).when(instance).hasInterface[PortsLike]
    }

    // Setup MultiBusLike interface
    if (hasMultiBusInterface) {
      val multiBusInterface = mock[MultiBusLike]
      doReturn(true).when(instance).hasInterface[MultiBusLike]
      doReturn(Some(multiBusInterface))
        .when(instance)
        .getInterface[MultiBusLike]

      // Setup bus retrieval methods
      doReturn(1).when(multiBusInterface).numberOfBuses

      if (isSupplier) {
        val supplierBus = mock[SupplierBusLike]
        doReturn("bus_").when(supplierBus).getPrefix
        doReturn(true).when(supplierBus).isSupplier
        doReturn(isHardware).when(supplierBus).isHardware

        // Use separate doReturn...when calls for different conditions
        if (busName.isEmpty) {
          doReturn(None)
            .when(multiBusInterface)
            .getFirstSupplierBusByName(anyString())
        } else {
          doReturn(Some(supplierBus))
            .when(multiBusInterface)
            .getFirstSupplierBusByName(busName)
        }

        if (busProtocol.isEmpty) {
          doReturn(None)
            .when(multiBusInterface)
            .getFirstSupplierBusOfProtocol(anyString())
        } else {
          doReturn(Some(supplierBus))
            .when(multiBusInterface)
            .getFirstSupplierBusOfProtocol(busProtocol)
        }
      } else {
        doReturn(None)
          .when(multiBusInterface)
          .getFirstSupplierBusByName(anyString())
        doReturn(None)
          .when(multiBusInterface)
          .getFirstSupplierBusOfProtocol(anyString())
      }

      // Setup consumer bus methods
      val consumerBus = mock[BusLike]

      // Use separate doReturn...when calls for different conditions
      if (busName.isEmpty) {
        doReturn(None)
          .when(multiBusInterface)
          .getFirstConsumerBusByName(anyString())
      } else {
        doReturn(Some(consumerBus))
          .when(multiBusInterface)
          .getFirstConsumerBusByName(anyString())
      }

      if (busProtocol.isEmpty) {
        doReturn(None)
          .when(multiBusInterface)
          .getFirstConsumerBusOfProtocol(anyString())
      } else {
        doReturn(Some(consumerBus))
          .when(multiBusInterface)
          .getFirstConsumerBusOfProtocol(anyString())
      }
    } else {
      doReturn(false).when(instance).hasInterface[MultiBusLike]
      doReturn(None).when(instance).getInterface[MultiBusLike]
    }

    // Setup RamLike interface for consumer instances
    if (!isSupplier) {
      val ramInterface = mock[RamLike]
      doReturn(true).when(instance).hasInterface[RamLike]
      doReturn(ramInterface).when(instance).getInterfaceUnwrapped[RamLike]

      // Setup RAM ranges
      val ramRanges = Seq(
        (BigInt(0x1000), BigInt(0x1000), true, Seq.empty[String])
      )
      doReturn(ramRanges).when(ramInterface).getRanges
    }

    // Setup isInstanceOf for ChipInstance
    doReturn(true).when(instance).isInstanceOf[ChipInstance]

    instance
  }

  // Helper method to create an InstanceLoc
  private def createInstanceLoc(
      instance: ChipInstance,
      fullName: String
  ): InstanceLoc = {
    InstanceLoc(instance, None, fullName)
  }

  // Test getBus() method with various input combinations
  "UnconnectedBus.getBus" should "find a supplier bus by name when specified" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true,
      busName = "main_bus",
      busProtocol = "axi4"
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false,
      busName = "mem_bus",
      busProtocol = "axi4"
    )

    // Create instance locations
    val supplierLoc = createInstanceLoc(supplierInstance, "cpu")
    val consumerLoc = createInstanceLoc(consumerInstance, "memory")

    // Create UnconnectedBus with specific bus name
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4",
      supplierBusName = "main_bus",
      consumerBusName = "mem_bus"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result should not be empty
    result.head shouldBe a[ConnectedBus]
    val connectedBus = result.head.asInstanceOf[ConnectedBus]
    connectedBus.firstFullName shouldBe "cpu"
    connectedBus.secondFullName shouldBe "memory"
  }

  it should "find a supplier bus by protocol when name is not specified" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true,
      busName = "main_bus",
      busProtocol = "axi4"
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false,
      busName = "mem_bus",
      busProtocol = "axi4"
    )

    // Create UnconnectedBus with only protocol specified
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4",
      supplierBusName = "",
      consumerBusName = ""
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result should not be empty
    result.head shouldBe a[ConnectedBus]
  }

  it should "return None when neither name nor protocol matches" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true,
      busName = "main_bus",
      busProtocol = "axi4"
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false,
      busName = "mem_bus",
      busProtocol = "axi4"
    )

    // Create UnconnectedBus with non-matching name and protocol
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "apb", // Different protocol
      supplierBusName = "other_bus", // Different name
      consumerBusName = ""
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result shouldBe empty
  }

  it should "return None when supplier doesn't have MultiBusLike interface" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      hasMultiBusInterface = false,
      isSupplier = true
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false
    )

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result shouldBe empty
  }

  it should "return None when supplier has no buses" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true
    )

    // Override the numberOfBuses method to return 0
    val multiBusInterface = supplierInstance.getInterface[MultiBusLike].get
    doReturn(0).when(multiBusInterface).numberOfBuses

    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false
    )

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result shouldBe empty
  }

  it should "handle bidirectional connections correctly" in {
    // Create test instances
    val instance1 = createMockChipInstance(
      name = "device1",
      isSupplier = true
    )
    val instance2 = createMockChipInstance(
      name = "device2",
      isSupplier = false
    )

    // Create UnconnectedBus with bidirectional connection
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.BiDirectional,
      secondFullName = "device2",
      busProtocol = "axi4"
    )

    // Test the connection
    val result = bus.connect(Seq(instance1, instance2))

    // Verify the result - should be empty because bidirectional bus connections are not supported
    result shouldBe empty
  }

  // Test preConnect() method for error handling
  "UnconnectedBus.preConnect" should "handle missing instances gracefully" in {
    // Create test instances
    val instance = createMockChipInstance(
      name = "device1",
      isSupplier = true
    )

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
    // Create test instances
    val instance1 = createMockChipInstance(
      name = "device1",
      isSupplier = true
    )
    val instance2 = createMockChipInstance(
      name = "device2",
      isSupplier = false
    )

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

  it should "handle instances without PortsLike interface" in {
    // Create test instances
    val instance1 = createMockChipInstance(
      name = "device1",
      hasPortsInterface = false,
      isSupplier = true
    )
    val instance2 = createMockChipInstance(
      name = "device2",
      isSupplier = false
    )

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "device1",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "device2",
      busProtocol = "axi4"
    )

    // Test preConnect with instance missing PortsLike interface
    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq(instance1, instance2))
      }
    }
  }

  it should "handle RAM instances correctly" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true
    )
    val ramInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false
    )

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4"
    )

    // Test preConnect with RAM instance
    withSilentLogs {
      noException should be thrownBy {
        bus.preConnect(Seq(supplierInstance, ramInstance))
      }
    }
  }

  // Test connect() method for different connection scenarios
  "UnconnectedBus.connect" should "create correct connections for hardware instances" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "fpga",
      isSupplier = true,
      isHardware = true
    )
    val consumerInstance = createMockChipInstance(
      name = "ddr",
      isSupplier = false,
      isHardware = true
    )

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

  it should "create both bus and port connections for non-hardware instances" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true,
      isHardware = false
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false,
      isHardware = false
    )

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result should not be empty
    result.head shouldBe a[ConnectedBus]

    // For non-hardware instances, we should have both bus and port connections
    result.length should be > 1
    result.tail.forall(_.isInstanceOf[ConnectedPortGroup]) shouldBe true
  }

  it should "handle silent mode correctly" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true
    )

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
      val result = silentBus.connect(Seq(supplierInstance))
      result shouldBe empty
    }
  }

  it should "handle SecondToFirst direction correctly" in {
    // Create test instances
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false
    )

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

  // Test finaliseBuses() method
  "UnconnectedBus.finaliseBuses" should "compute consumer addresses correctly" in {
    // Create test instances with mocked SupplierBusLike
    val supplierInstance = createMockChipInstance(
      name = "cpu",
      isSupplier = true
    )
    val consumerInstance = createMockChipInstance(
      name = "memory",
      isSupplier = false
    )

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "cpu",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "memory",
      busProtocol = "axi4"
    )

    // Test finaliseBuses
    withSilentLogs {
      noException should be thrownBy {
        bus.finaliseBuses(Seq(supplierInstance, consumerInstance))
      }
    }

    // Verify that computeConsumerAddresses was called
    val multiBusInterface = supplierInstance.getInterface[MultiBusLike].get
    val supplierBus =
      multiBusInterface.getFirstSupplierBusOfProtocol("axi4").get
    // Use verify with doCallRealMethod to avoid Mockito issues
    doCallRealMethod().when(supplierBus).computeConsumerAddresses()
    bus.finaliseBuses(Seq(supplierInstance, consumerInstance))
    verify(supplierBus, times(1)).computeConsumerAddresses()
  }
}
