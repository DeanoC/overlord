package com.deanoc.overlord.instances

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import com.deanoc.overlord._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.config._
import com.deanoc.overlord.interfaces.{RamLike, BusLike}
import com.deanoc.overlord.definitions._

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

  override def withFixture(test: NoArgTest) = {
    Overlord.setInstancePath("test_instance") // Push to instanceStack
    try {
      super.withFixture(test)
    } finally {
      Overlord.popInstancePath() // Pop from instanceStack to clean up
    }
  }

  "RamInstance" should "be created with type-safe configuration" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[HardwareDefinition]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[RamDefinitionConfig]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("ram", "sram"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Create a type-safe RamConfig with memory ranges
    val ramConfig = RamDefinitionConfig(
      `type` = "ram.sram",
      ranges = List(
        MemoryRangeConfig(
          address = "0x10000000",
          size = "0x1000"
        ),
        MemoryRangeConfig(
          address = "0x20000000",
          size = "0x2000"
        )
      ),
      attributes = Map.empty
    )

    // Mock the behavior of mockDefinition.config to return the ramConfig
    when(mockDefinition.config).thenReturn(ramConfig)

    // Create the RamInstance
    val ramInstance = RamInstance(
      name = "test_ram",
      definition = mockDefinition
    )

    // Verify the instance properties
    ramInstance.name shouldBe "test_ram"
    ramInstance.definition shouldBe mockDefinition

    // Verify the memory ranges
    ramInstance.getRanges should have size 2

    val range1 = ramInstance.getRanges(0)
    range1._1 shouldBe BigInt("10000000", 16)
    range1._2 shouldBe BigInt("1000", 16)

    val range2 = ramInstance.getRanges(1)
    range2._1 shouldBe BigInt("20000000", 16)
    range2._2 shouldBe BigInt("2000", 16)
  }

  it should "handle bus specifications correctly" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[HardwareDefinition]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[RamDefinitionConfig]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("ram", "dram"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Create a bus specification in the attributes
    val busSpec = Map(
      "name" -> "mem_bus",
      "supplier" -> false,
      "protocol" -> "axi",
      "prefix" -> "s_axi",
      "data_width" -> 32,
      "address_width" -> 32
    )
    val busesArray = Utils.toVariant(Seq(busSpec))
    val ranges = Utils.toVariant(List(
      Map(
        "address" -> "0x30000000",
        "size" -> "0x10000"
      )
    ))
    val attributes = Map[String, Variant](
      "buses" -> busesArray,
      "ranges" -> ranges
    )
    when(mockConfig.attributesAsVariant).thenReturn(attributes)

    // Create the RamInstance
    val ramInstance = RamInstance(
      name = "dram_instance",
      definition = mockDefinition
    )

    // Verify the bus interface
    ramInstance.numberOfBuses shouldBe 1

    val bus0 = ramInstance.getBus(0)
    bus0 shouldBe defined
    bus0.get.toString should include("mem_bus")

    val axiBus = ramInstance.getFirstConsumerBusOfProtocol("axi")
    axiBus shouldBe defined
    axiBus.get.toString should include("axi")

    ramInstance.getFirstSupplierBusOfProtocol("axi") shouldBe None
  }

  it should "calculate total memory size correctly" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[HardwareDefinition]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[RamDefinitionConfig]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("ram", "sram"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Create ranges for the memory config
    val ranges = List(
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
    val attributes = Map[String, Variant]()
    when(mockConfig.attributes).thenReturn(attributes)
    when(mockConfig.attributesAsVariant).thenReturn(attributes)
    when(mockConfig.ranges).thenReturn(ranges)
     

    // Create the RamInstance
    val ramInstance = RamInstance(
      name = "multi_range_ram",
      definition = mockDefinition
    )

    // Verify the total memory size
    val totalSize = ramInstance.getRanges.map(_._2).sum
    totalSize shouldBe BigInt("6000", 16)
  }

  it should "handle empty memory ranges gracefully" in {
    // Create a mock ChipDefinitionTrait
    val mockDefinition = mock[HardwareDefinition]
    val mockDefType = mock[DefinitionType]
    val mockConfig = mock[RamDefinitionConfig]

    // Set up the mock definition
    when(mockDefinition.defType).thenReturn(mockDefType)
    when(mockDefType.ident).thenReturn(Seq("ram", "sram"))
    when(mockDefinition.config).thenReturn(mockConfig)

    // Create attributes map with empty ranges
    val ranges = List[MemoryRangeConfig]();
    val attributes = Map[String, Variant]()
    when(mockConfig.attributesAsVariant).thenReturn(attributes)
    when(mockConfig.ranges).thenReturn(ranges)

    // Create the RamInstance
    val ramInstance = RamInstance(
      name = "empty_ram",
      definition = mockDefinition
    )

    // Verify the memory ranges
    ramInstance.getRanges shouldBe empty
    ramInstance.getRanges.size shouldBe 0
    ramInstance.getRanges.map(_._2).sum shouldBe BigInt(0)
  }
}
