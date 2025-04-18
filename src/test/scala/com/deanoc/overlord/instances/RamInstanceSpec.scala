package com.deanoc.overlord.instances

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.config.{RamConfig, MemoryRangeConfig}
import com.deanoc.overlord.interfaces.{RamLike, BusLike}

/** Test suite for the RamInstance class.
  *
  * This test suite demonstrates the use of type-safe configuration classes and
  * dependency injection for improved testability.
  */
class RamInstanceSpec
    extends AnyFlatSpec
    with Matchers
    with MockitoSugar
    with SilentLogger {

  "RamInstance" should "be created with type-safe configuration" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("ram", "sram"))

    // Create attributes map for the definition
    val attributes = Map[String, Variant]()
    when(mockDefinition.attributes).thenReturn(attributes)

    // Create a type-safe RamConfig with memory ranges
    val ramConfig = RamConfig(
      ranges = List(
        MemoryRangeConfig(
          address = "0x10000000",
          size = "0x1000"
        ),
        MemoryRangeConfig(
          address = "0x20000000",
          size = "0x2000"
        )
      )
    )

    // Create the RamInstance with the injected configuration
    val ramInstance = new RamInstance(
      name = "test_ram",
      definition = mockDefinition,
      config = ramConfig
    )

    // Verify the instance properties
    ramInstance.name shouldBe "test_ram"
    ramInstance.definition shouldBe mockDefinition

    // Verify the memory ranges from the config
    ramInstance.getRanges should have size 2

    // Verify the first memory range - use tuple accessors instead of properties
    val range1 = ramInstance.getRanges(0)
    range1._1 shouldBe BigInt("10000000", 16) // Use tuple index _1 for address
    range1._2 shouldBe BigInt("1000", 16) // Use tuple index _2 for size

    // Verify the second memory range - use tuple accessors instead of properties
    val range2 = ramInstance.getRanges(1)
    range2._1 shouldBe BigInt("20000000", 16) // Use tuple index _1 for address
    range2._2 shouldBe BigInt("2000", 16) // Use tuple index _2 for size

    // Verify the RamLike interface
    val ramInterface = ramInstance.getInterface[RamLike]
    ramInterface shouldBe defined

    // Verify the memory range methods - use getRanges methods instead of direct property access
    ramInstance.getRanges.size shouldBe 2 // Use .size instead of getMemoryRangeCount
    ramInstance.getRanges(
      0
    ) shouldBe range1 // Use getRanges(index) instead of getMemoryRange
    ramInstance.getRanges(
      1
    ) shouldBe range2 // Use getRanges(index) instead of getMemoryRange
  }

  it should "handle bus specifications correctly" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("ram", "dram"))

    // Create a bus specification in the attributes
    val busSpec = Utils.toVariant(
      Map(
        "name" -> "mem_bus",
        "supplier" -> false,
        "protocol" -> "axi",
        "prefix" -> "s_axi",
        "data_width" -> 32,
        "address_width" -> 32
      )
    )

    val busesArray = Utils.toVariant(Array(busSpec))

    val attributes = Map[String, Variant](
      "buses" -> busesArray
    )
    when(mockDefinition.attributes).thenReturn(attributes)

    // Create a type-safe RamConfig
    val ramConfig = RamConfig(
      ranges = List(
        MemoryRangeConfig(
          address = "0x30000000",
          size = "0x10000"
        )
      )
    )

    // Create the RamInstance with the injected configuration
    val ramInstance = new RamInstance(
      name = "dram_instance",
      definition = mockDefinition,
      config = ramConfig
    )

    // Verify the bus interface
    ramInstance.numberOfBuses shouldBe 1

    // Verify we can get the bus by index
    val bus0 = ramInstance.getBus(0)
    bus0 shouldBe defined
    bus0.get.toString should include(
      "mem_bus"
    ) // Changed to use toString instead of getBusName

    // Verify we can get the bus by protocol
    val axiBus = ramInstance.getFirstConsumerBusOfProtocol("axi")
    axiBus shouldBe defined
    axiBus.get.toString should include(
      "axi"
    ) // Changed to use toString instead of getProtocol

    // Verify the bus is a consumer bus (not a supplier)
    ramInstance.getFirstSupplierBusOfProtocol("axi") shouldBe None
  }

  it should "calculate total memory size correctly" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("ram", "sram"))

    // Create attributes map for the definition
    val attributes = Map[String, Variant]()
    when(mockDefinition.attributes).thenReturn(attributes)

    // Create a type-safe RamConfig with multiple memory ranges
    val ramConfig = RamConfig(
      ranges = List(
        MemoryRangeConfig(
          address = "0x10000000",
          size = "0x1000"
        ),
        MemoryRangeConfig(
          address = "0x20000000",
          size = "0x2000"
        ),
        MemoryRangeConfig(
          address = "0x30000000",
          size = "0x3000"
        )
      )
    )

    // Create the RamInstance with the injected configuration
    val ramInstance = new RamInstance(
      name = "multi_range_ram",
      definition = mockDefinition,
      config = ramConfig
    )

    // Verify the total memory size
    // 0x1000 + 0x2000 + 0x3000 = 0x6000 = 24576 bytes
    val totalSize =
      ramInstance.getRanges.map(_._2).sum // Calculate sum directly
    totalSize shouldBe BigInt("6000", 16)
  }

  it should "handle empty memory ranges gracefully" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[ChipDefinitionTrait]
    val mockDefType = mock[DefinitionType]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Array("ram", "sram"))

    // Create attributes map for the definition
    val attributes = Map[String, Variant]()
    when(mockDefinition.attributes).thenReturn(attributes)

    // Create a type-safe RamConfig with no memory ranges
    val ramConfig = RamConfig(
      ranges = List()
    )

    // Create the RamInstance with the injected configuration
    val ramInstance = new RamInstance(
      name = "empty_ram",
      definition = mockDefinition,
      config = ramConfig
    )

    // Verify the memory ranges
    ramInstance.getRanges should be(
      empty
    ) // Use getRanges instead of memoryRanges
    ramInstance.getRanges.size shouldBe 0 // Use getRanges.size instead of getMemoryRangeCount
    ramInstance.getRanges.map(_._2).sum shouldBe BigInt(
      0
    ) // Calculate sum directly
  }
}
