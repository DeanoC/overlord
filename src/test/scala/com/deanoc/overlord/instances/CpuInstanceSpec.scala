package com.deanoc.overlord.instances

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.interfaces.{MultiBusLike, SupplierBusLike, BusLike}
import com.deanoc.overlord.definitions._
import com.deanoc.overlord.config._

/** Test suite for the CpuInstance class.
  *
  * This test suite demonstrates the use of type-safe configuration classes and
  * dependency injection for improved testability.
  */
class CpuInstanceSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  override def withFixture(test: NoArgTest) = {
    Overlord.setInstancePath("test_instance") // Push to instanceStack
    try {
      super.withFixture(test)
    } finally {
      Overlord.popInstancePath() // Pop from instanceStack to clean up
    }
  }

  "CpuInstance" should "be created with type-safe configuration" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[CpuDefinitionConfig]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("cpu", "riscv", "core"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Create attributes map for the definition
    // Add required CPU-specific attributes that may be missing
    val attributes = Map[String, Variant](
      "width" -> Utils.toVariant(32),
      "max_atomic_width" -> Utils.toVariant(64),
      "max_bitop_type_width" -> Utils.toVariant(32),
      "gcc_flags" -> Utils.toVariant("-march=rv32imac"),
      "stack_size" -> Utils.toVariant("0x1000"),
      "heap_size" -> Utils.toVariant("0x2000"),
      "cpu_type" -> Utils.toVariant("riscv"),
      "core_count" -> Utils.toVariant(4),
      "triple" -> Utils.toVariant("riscv32-unknown-elf")
    )
    when(mockConfig.attributes).thenReturn(attributes)
    when(mockConfig.triple).thenReturn("riscv32-unknown-elf")
    when(mockConfig.core_count).thenReturn(4)

    // Create the CpuInstance with the refactored structure
    val cpuInstance = CpuInstance("test_cpu", mockDefinition)

    // Verify the instance properties derived from the config
    cpuInstance.name shouldBe "test_cpu"
    cpuInstance.definition shouldBe mockDefinition
    cpuInstance.triple shouldBe "riscv32-unknown-elf"
    cpuInstance.cpuCount shouldBe 4
    cpuInstance.sanitizedTriple shouldBe "riscv32_unknown_elf"

    // Verify the instance properties derived from the attributes
    cpuInstance.width shouldBe 32
    cpuInstance.maxAtomicWidth shouldBe 64
    cpuInstance.maxBitOpTypeWidth shouldBe 32
    cpuInstance.gccFlags shouldBe "-march=rv32imac"

    // Verify the CPU type is derived correctly
    cpuInstance.cpuType shouldBe "riscv"
    cpuInstance.host shouldBe false
  }

  it should "handle host CPU type correctly" in {
    // Create a mock ChipDefinitionTrait for a host CPU
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[CpuDefinitionConfig]

    // Set up the mock definition for a host CPU
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("cpu", "x86", "host"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Create a minimal attributes map with necessary fields
    val attributes = Map[String, Variant](
      "width" -> Utils.toVariant(64), // Add CPU width
      "stack_size" -> Utils.toVariant("0x2000"), // Add stack size
      "heap_size" -> Utils.toVariant("0x4000"), // Add heap size
      "cpu_type" -> Utils.toVariant("host"), // Explicit CPU type
      "core_count" -> Utils.toVariant(8),
      "triple" -> Utils.toVariant("x86_64-unknown-linux-gnu")
    )
    when(mockConfig.attributes).thenReturn(attributes)
    when(mockConfig.triple).thenReturn("x86_64-unknown-linux-gnu")
    when(mockConfig.core_count).thenReturn(8)

    // Create the CpuInstance
    val cpuInstance = CpuInstance("host_cpu", mockDefinition)

    // Verify the host property is true
    cpuInstance.host shouldBe true
    cpuInstance.cpuType shouldBe "host"
  }

  it should "handle bus specifications correctly" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[CpuDefinitionConfig]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("cpu", "arm", "cortex"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Instead of an Array of Variant objects, use a Seq/List of Maps
    val busSpecs = List(
      Map(
        "name" -> "data_bus",
        "supplier" -> true,
        "protocol" -> "axi",
        "prefix" -> "m_axi",
        "base_address" -> "0x40000000",
        "data_width" -> 32,
        "address_width" -> 32,
        "fixed_base_address" -> false
      ),
      Map(
        "name" -> "instruction_bus",
        "supplier" -> true,
        "protocol" -> "axi",
        "prefix" -> "m_axi_instr",
        "base_address" -> "0x00000000",
        "data_width" -> 32,
        "address_width" -> 32,
        "fixed_base_address" -> true
      )
    )
    val busesArray = Utils.toVariant(busSpecs)

    val attributes = Map[String, Variant](
      "buses" -> busesArray,
      "width" -> Utils.toVariant(32), // Add CPU width
      "stack_size" -> Utils.toVariant("0x1000"), // Add stack size
      "heap_size" -> Utils.toVariant("0x2000"), // Add heap size
      "cpu_type" -> Utils.toVariant("arm") // Explicit CPU type
    )
    when(mockConfig.attributesAsVariant).thenReturn(Utils.toVariant(attributes))
    when(mockConfig.attributes).thenReturn(attributes)
    when(mockConfig.core_count).thenReturn(2)
    when(mockConfig.triple).thenReturn("arm-none-eabi")

    // Create a type-safe CpuConfig
    val cpuConfig = CpuDefinitionConfig(
      name = "arm_cpu",
      `type` = "cpu.arm.test",
      triple = "arm-none-eabi",
      core_count = 2,
    )

    // Create the CpuInstance with the refactored structure
    val cpuInstance = CpuInstance("arm_cpu", mockDefinition)

    // Verify the MultiBusLike interface
    cpuInstance.numberOfBuses shouldBe 2

    // Verify we can get the bus by index
    val bus0 = cpuInstance.getBus(0)
    bus0 shouldBe defined
    // Verify we can get the bus by name
    val dataBus = cpuInstance.getFirstSupplierBusByName("data_bus")
    dataBus shouldBe defined
    dataBus.get.toString should include(
      "data_bus"
    ) // Changed to use toString instead of getBusName
    // Verify we can get the bus by protocol
    val axiBus = cpuInstance.getFirstSupplierBusOfProtocol("axi")
    axiBus shouldBe defined
    // Verify the interface casting works
    val multiBusInterface = cpuInstance.getInterface[MultiBusLike]
    multiBusInterface shouldBe defined
  }

  it should "handle error cases gracefully" in {
    // Create a mock ChipDefinitionTrait that will cause an error
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockConfig = mock[CpuDefinitionConfig]

    when(mockDefinition.config).thenReturn(mockConfig)

    // Set up the mock to throw an exception when attributes are accessed
    when(mockConfig.attributes).thenThrow(
      new RuntimeException("Test exception")
    )

    // Create the CpuInstance with the refactored structure
    try {
      val cpuInstance = CpuInstance("error_cpu", mockDefinition)
      fail("Expected exception was not thrown")
    } catch {
      case ex: Exception =>
        ex.getMessage should include("Test exception")
    }
  }
}
