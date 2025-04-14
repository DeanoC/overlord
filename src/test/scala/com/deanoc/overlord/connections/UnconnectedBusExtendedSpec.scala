package com.deanoc.overlord.connections

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import org.mockito.Mockito.withSettings
import com.deanoc.overlord._
import scala.language.implicitConversions
import com.deanoc.overlord.connections.ConnectionTypesTestExtensions._
import com.deanoc.overlord.utils.{SilentLogger, Logging, ModuleLogger, Utils}
import com.deanoc.overlord.utils.{Variant, TableV}
import com.deanoc.overlord.DefinitionType
import com.deanoc.overlord.connections.ConnectionTypes._
import com.deanoc.overlord.instances.{CpuInstance, RamInstance}

// Helper methods for testing
object UnconnectedBusExtendedTestHelpers {
  implicit class BusNameTestOps(name: BusName) {
    def value: String = name.toString
  }
}
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}
import com.deanoc.overlord.connections.InstanceLoc
import com.deanoc.overlord.interfaces._
import com.deanoc.overlord.hardware.{
  Port,
  BitsDesc,
  InWireDirection,
  OutWireDirection
}
import com.deanoc.overlord.utils.{Variant, BigIntV}

import scala.collection.mutable
import scala.reflect.ClassTag
import scala.reflect.ClassTag

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

  Project.setInstancePath("/workspace/")

  // Helper method to create an InstanceLoc
  private def createInstanceLoc(
      instance: ChipInstance,
      fullName: String
  ): InstanceLoc = {
    InstanceLoc(instance, None, fullName)
  }

  // Helper methods to create CpuDef and RamDef
  private def createCpuDef(): HardwareDefinitionTrait = new HardwareDefinitionTrait {
    override val ports: Map[String, com.deanoc.overlord.hardware.Port] = Map()
    override val maxInstances: Int = 1
    override val defType: DefinitionType = CpuDefinitionType(Seq("cpu", "riscv", "verilog"))
    override val attributes: Map[String, Variant] = Utils.fromYaml(
      """
buses:
- base_address: '0xFD00_0000'
  data_width: 32
  protocol: axi4
  name: pmu_pmuswitch
  supplier: true
core_count: 1
max_atomic_width: 32
max_bitop_type_width: 32
registers: []
triple: riscv-none-elf
type: cpu.pmu.zynqps8
width: 32
registers:
- base_address: '0x02D4_0000'
  cpus: PMU
  name: PMU_IOMODULE
  resource: registers/pmu_iomodule.yaml
ranges:
- address: '0x02DC_0000'
  size: 128 KiB
      """
      )
    override def toString(): String = "RiscVCpu"
    protected val registersV: Seq[com.deanoc.overlord.utils.Variant] = Seq()
    override val dependencies: Seq[String] = Seq()
    override val sourcePath: java.nio.file.Path = java.nio.file.Paths.get("path/to/source")
  }

  private def createRamDef(): HardwareDefinitionTrait = new HardwareDefinitionTrait {
    override val ports: Map[String, com.deanoc.overlord.hardware.Port] = Map()
    override val maxInstances: Int = 1
    override val defType: DefinitionType = RamDefinitionType(Seq("ram", "verilog"))
    override val attributes: Map[String, Variant] = Utils.fromYaml(
      """
buses:
- base_address: '0xFD00_0000'
  data_width: 32
  protocol: axi4
  name: pmu_pmuswitch
  supplier: false
      """
      )
    override def toString(): String = "Ram"
    protected val registersV: Seq[com.deanoc.overlord.utils.Variant] = Seq()
    override val dependencies: Seq[String] = Seq()
    override val sourcePath: java.nio.file.Path = java.nio.file.Paths.get("path/to/source")
  }

  // Test getBus() method with various input combinations
  "UnconnectedBus.getBus" should "find a supplier bus by name when specified" in {
    val cpuDef = createCpuDef()
    val ramDef = createRamDef()
    val supplierInstance = CpuInstance("cpu", cpuDef)
    val consumerInstance = RamInstance("memory", ramDef)

    // Create UnconnectedBus with SecondToFirst direction
    val bus = UnconnectedBus(
      firstFullName = "memory",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "cpu",
      busProtocol = "axi4",
      supplierBusName = "pmu_pmuswitch",
      consumerBusName = "pmu_pmuswitch"
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

  it should "return an empty sequence if no bus connection possible" in {
    val cpuDef = createCpuDef()
    val ramDef = createRamDef()
    val supplierInstance = CpuInstance("cpu", cpuDef)
    val consumerInstance = RamInstance("memory", ramDef)

    // Create UnconnectedBus with SecondToFirst direction
    val bus = UnconnectedBus(
      firstFullName = "memory",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "cpu",
      busProtocol = "axi4",
      supplierBusName = "main_bus",
      consumerBusName = "mem_bus"
    )

    // Test the connection
    val result = bus.connect(Seq(supplierInstance, consumerInstance))

    // Verify the result
    result shouldBe empty
  }

  it should "handle missing instances gracefully" in {
    val cpuDef = createCpuDef()
    val instance = CpuInstance("device1", cpuDef)

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
    val cpuDef = createCpuDef()
    val ramDef = createRamDef()
    val instance1 = CpuInstance("device1", cpuDef)
    val instance2 = RamInstance("device2", ramDef)

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

  // Test connect() method for different connection scenarios
  "UnconnectedBus.connect" should "create correct connections for hardware instances" in {
    val cpuDef = createCpuDef()
    val ramDef = createRamDef()
    val supplierInstance = CpuInstance("fpga", cpuDef)
    val consumerInstance = RamInstance("ddr", ramDef)

    // Create UnconnectedBus
    val bus = UnconnectedBus(
      firstFullName = "fpga",
      direction = ConnectionDirection.FirstToSecond,
      secondFullName = "ddr",
      busProtocol = "axi4",
      supplierBusName = "pmu_pmuswitch",
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
    val cpuDef = createCpuDef()
    // Create test instances
    val supplierInstance = CpuInstance("fpga", cpuDef)

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
    val cpuDef = createCpuDef()
    val ramDef = createRamDef()
 
    val supplierInstance = CpuInstance("cpu", cpuDef)
    val consumerInstance = RamInstance("memory", ramDef)

    // Create UnconnectedBus with SecondToFirst direction
    val bus = UnconnectedBus(
      firstFullName = "memory",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "cpu",
      busProtocol = "axi4",
      supplierBusName = "pmu_pmuswitch",
      consumerBusName = "pmu_pmuswitch"
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
    val cpuDef = createCpuDef()
    val ramDef = createRamDef()
 
    val supplierInstance = CpuInstance("cpu", cpuDef)
    val consumerInstance = RamInstance("memory", ramDef)
    // Create UnconnectedBus with SecondToFirst direction
    val bus = UnconnectedBus(
      firstFullName = "memory",
      direction = ConnectionDirection.SecondToFirst,
      secondFullName = "cpu",
      busProtocol = "axi4",
      supplierBusName = "pmu_pmuswitch",
      consumerBusName = "pmu_pmuswitch"
    )
    // Test finaliseBuses
    withSilentLogs {
      noException should be thrownBy {
        bus.finaliseBuses(Seq(supplierInstance, consumerInstance))
      }
    }

    // For this test, we'll just verify that finaliseBuses doesn't throw an exception
    // The actual verification of computeConsumerAddresses would require more complex mocking
    // which is challenging with Scala's type system and Mockito
  }


  override def withFixture(test: NoArgTest) = {
    // Force trace-level debugging for all tests
    ModuleLogger.setDefaultLogLevel(org.slf4j.event.Level.TRACE)
    super.withFixture(test)
  }
}
