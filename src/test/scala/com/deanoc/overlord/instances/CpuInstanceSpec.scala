package com.deanoc.overlord.instances

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.config.CpuConfig
import com.deanoc.overlord.interfaces.{MultiBusLike, SupplierBusLike, BusLike}

/**
 * Test suite for the CpuInstance class.
 * 
 * This test suite demonstrates the use of type-safe configuration classes
 * and dependency injection for improved testability.
 */
class CpuInstanceSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {
  
  "CpuInstance" should "be created with type-safe configuration" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]
    
    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("cpu", "riscv", "core"))
    
    // Create attributes map for the definition
    val attributes = Map[String, Variant](
      "width" -> Utils.toVariant(32),
      "max_atomic_width" -> Utils.toVariant(64),
      "max_bitop_type_width" -> Utils.toVariant(32),
      "gcc_flags" -> Utils.toVariant("-march=rv32imac")
    )
    when(mockDefinition.attributes).thenReturn(attributes)
    
    // Create a type-safe CpuConfig
    val cpuConfig = CpuConfig(
      core_count = 4,
      triple = "riscv32-unknown-elf"
    )
    
    // Create the CpuInstance with the injected configuration
    val cpuInstanceResult = CpuInstance("test_cpu", mockDefinition, cpuConfig)
    
    // Verify the result is Right and contains a CpuInstance
    cpuInstanceResult.isRight shouldBe true
    val cpuInstance = cpuInstanceResult.toOption.get
    
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
    
    // Set up the mock definition for a host CPU
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("cpu", "x86", "host"))
    
    // Create a minimal attributes map
    val attributes = Map[String, Variant]()
    when(mockDefinition.attributes).thenReturn(attributes)
    
    // Create a type-safe CpuConfig
    val cpuConfig = CpuConfig(
      core_count = 8,
      triple = "x86_64-unknown-linux-gnu"
    )
    
    // Create the CpuInstance with the injected configuration
    val cpuInstanceResult = CpuInstance("host_cpu", mockDefinition, cpuConfig)
    
    // Verify the result is Right and contains a CpuInstance
    cpuInstanceResult.isRight shouldBe true
    val cpuInstance = cpuInstanceResult.toOption.get
    
    // Verify the host property is true
    cpuInstance.host shouldBe true
    cpuInstance.cpuType shouldBe "host"
  }
  
  it should "handle bus specifications correctly" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]
    
    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("cpu", "arm", "cortex"))
    
    // Create a bus specification in the attributes
    val busSpec1 = Utils.toVariant(Map(
      "name" -> "data_bus",
      "supplier" -> true,
      "protocol" -> "axi",
      "prefix" -> "m_axi",
      "base_address" -> "0x40000000",
      "data_width" -> 32,
      "address_width" -> 32,
      "fixed_base_address" -> false
    ))
    
    val busSpec2 = Utils.toVariant(Map(
      "name" -> "instruction_bus",
      "supplier" -> true,
      "protocol" -> "axi",
      "prefix" -> "m_axi_instr",
      "base_address" -> "0x00000000",
      "data_width" -> 32,
      "address_width" -> 32,
      "fixed_base_address" -> true
    ))
    
    val busesArray = Utils.toVariant(Array(busSpec1, busSpec2))
    
    val attributes = Map[String, Variant](
      "buses" -> busesArray
    )
    when(mockDefinition.attributes).thenReturn(attributes)
    
    // Create a type-safe CpuConfig
    val cpuConfig = CpuConfig(
      core_count = 2,
      triple = "arm-none-eabi"
    )
    
    // Create the CpuInstance with the injected configuration
    val cpuInstanceResult = CpuInstance("arm_cpu", mockDefinition, cpuConfig)
    
    // Verify the result is Right and contains a CpuInstance
    cpuInstanceResult.isRight shouldBe true
    val cpuInstance = cpuInstanceResult.toOption.get
    
    // Verify the MultiBusLike interface
    cpuInstance.numberOfBuses shouldBe 2
    
    // Verify we can get the bus by index
    val bus0 = cpuInstance.getBus(0)
    bus0 shouldBe defined
    bus0.get.name shouldBe "data_bus"
    
    // Verify we can get the bus by name
    val dataBus = cpuInstance.getFirstSupplierBusByName("data_bus")
    dataBus shouldBe defined
    dataBus.get.spec.protocol shouldBe "axi"
    
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
    
    // Set up the mock to throw an exception when attributes are accessed
    when(mockDefinition.attributes).thenThrow(new RuntimeException("Test exception"))
    
    // Create a type-safe CpuConfig
    val cpuConfig = CpuConfig(
      core_count = 1,
      triple = "test-triple"
    )
    
    // Create the CpuInstance with the injected configuration
    val cpuInstanceResult = CpuInstance("error_cpu", mockDefinition, cpuConfig)
    
    // Verify the result is Left and contains an error message
    cpuInstanceResult.isLeft shouldBe true
    cpuInstanceResult.left.toOption.get should include("Error creating CpuInstance")
  }
}