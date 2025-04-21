package com.deanoc.overlord

import com.deanoc.overlord.{CatalogLoader, ComponentParser}
import com.deanoc.overlord.definitions.{DefinitionType, DefinitionTrait, SoftwareDefinitionTrait, Definition, HardwareDefinitionTrait}

import com.deanoc.overlord.utils._ // Import all members from utils
import com.deanoc.overlord.config._
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import org.scalatest.BeforeAndAfter
import org.scalatest.BeforeAndAfterAll
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.mockito.ArgumentMatchers.{any, eq => mockitoEq}
import io.circe.parser._
import io.circe.parser.{parse => jsonParse}
import io.circe.yaml.parser.{parse => yamlParse}
import io.circe.{Json, Decoder}

import java.nio.file.{Files, Path, Paths}
import java.io.{File, FileWriter}
import scala.collection.mutable

class ComponentParserTest
    extends AnyFlatSpec
    with Matchers
    with BeforeAndAfter
    with BeforeAndAfterAll {
  // Mock objects
  val mockCatalogs = mock(classOf[DefinitionCatalog])
  var parser: ComponentParser = _

  // Create temp directory for test files - shared across all tests
  var tempDir: Path = _

  override def beforeAll(): Unit = {
    // Create temp directory once before all tests
    tempDir = Files.createTempDirectory("overlord_test_")
    // create a valid catalog file for testing
  }

  before {
    // Create a fresh parser before each test
    parser = new ComponentParser()

    Overlord.resetPaths()
    // Add a default catalog path for testing
    Overlord.pushCatalogPath(tempDir)
  }

  "Parse Definition Catalog" should "correctly parse catalog file and extract definition data" in {
    // Load the YAML content from the actual test file
    val catalogFilePath = Paths.get("src/test/resources/test_catalog.yaml")
    val catalogYamlContent = new String(Files.readAllBytes(catalogFilePath))

    // Parse the YAML content into a CatalogFileConfig using circe
    val parsedYaml = yamlParse(catalogYamlContent).getOrElse(Json.Null)
    parsedYaml should not be Json.Null

    val catalogConfig = parsedYaml.as[CatalogFileConfig] match {
      case Right(config) => config
      case Left(error) => 
        fail(s"Failed to parse catalog file: ${error.message}, at path: ${error.history}")
    }
    
    // Assert that parsing was successful and the top-level structure is as expected
    catalogConfig.defaults should not be empty
    catalogConfig.definitions should not be empty
    catalogConfig.catalogs should not be empty
    
    // Extract and assert the defaults
    catalogConfig.defaults should have size 1
    catalogConfig.defaults.get("core_count") shouldBe Some("1")
    
    // Extract and assert the definitions
    catalogConfig.definitions should have size 5
    
    // Assert the content of the CPU definition
    val cpuDefinition = catalogConfig.definitions(0)
    cpuDefinition.`type` shouldBe "cpu"
    cpuDefinition.name shouldBe "Cortex-A53"
    
    // Cast to the specific CPU definition type to access CPU-specific fields
    cpuDefinition match {
      case cpu: CpuDefinitionConfig =>
        cpu.core_count shouldBe 4
        cpu.triple shouldBe "aarch64-none-elf"
      case _ => fail("Expected CPU definition to be of type CpuDefinitionConfig")
    }
    
    // Assert the content of the RAM definition
    val ramDefinition = catalogConfig.definitions(1)
    ramDefinition.`type` shouldBe "ram"
    ramDefinition.name shouldBe "MainMemory"
    
    // Check RAM config and ranges if supported by the schema
    ramDefinition match {
      case ram: RamDefinitionConfig =>
        ram.ranges should not be empty
      case _ =>
        ramDefinition.attributes.get("ranges") should not be None
    }
    
    // Assert the content of the clock definition
    val clockDefinition = catalogConfig.definitions(2)
    clockDefinition.`type` shouldBe "clock"
    clockDefinition.name shouldBe "SystemClock"
    clockDefinition.attributes.get("frequency") shouldBe Some("100MHz")
    
    // Assert the content of the IO definition
    val ioDefinition = catalogConfig.definitions(3)
    ioDefinition.`type` shouldBe "io"
    ioDefinition.name shouldBe "GPIO"
    ioDefinition.attributes.get("visible_to_software") shouldBe Some("true")
    
    // Assert the content of the pingroup definition
    val pinGroupDefinition = catalogConfig.definitions(4)
    pinGroupDefinition.`type` shouldBe "pingroup"
    pinGroupDefinition.name shouldBe "ControlPins"
    pinGroupDefinition.attributes.get("pins") should not be None
    pinGroupDefinition.attributes.get("direction") shouldBe Some("output")
    
    // Verify catalogs
    catalogConfig.catalogs should have size 3
    catalogConfig.catalogs(0).`type` shouldBe SourceType.Git
    catalogConfig.catalogs(0).url shouldBe Some("https://github.com/example/hardware-catalog.git")
    catalogConfig.catalogs(1).`type` shouldBe SourceType.Local
    catalogConfig.catalogs(1).path shouldBe Some("/path/to/local/catalog")
    catalogConfig.catalogs(2).`type` shouldBe SourceType.Fetch
    catalogConfig.catalogs(2).url shouldBe Some("https://example.com/api/catalog")
    
  }

  "Parse Project File" should "correctly parse project file with new format" in {
    val projectFilePath = Paths.get("src/test/resources/test_project.yaml")
    val projectYamlContent = new String(Files.readAllBytes(projectFilePath))

    // Parse the YAML content into a ProjectFileConfig using circe
    val parsedYaml = yamlParse(projectYamlContent).getOrElse(Json.Null)
    parsedYaml should not be Json.Null

    val projectConfig = parsedYaml.as[ComponentFileConfig] match {
        case Right(config) => config
        case Left(error) => 
          fail(s"Failed to parse project file: ${error.message}, at path: ${error.history}")
      }
    // Assert that parsing was successful and the top-level structure is as expected
    projectConfig.info should not be null
    projectConfig.components should not be empty
    projectConfig.connections should not be empty
    
    // Verify project info
    projectConfig.info.name shouldBe "example.soc"
    projectConfig.info.version should not be None
    projectConfig.info.author should not be None
    projectConfig.info.description should not be None

    projectConfig.info.version.get shouldBe "1.0.0"
    projectConfig.info.description.get shouldBe "Example SoC design with RISC-V cores"
    projectConfig.info.author.get shouldBe "Overlord Development Team"
    
    // Verify components
    projectConfig.components should have size 3
    
    // Check component entries
    val localComponent = projectConfig.components.find(_.`type` == SourceType.Local).get
    localComponent.path shouldBe Some("/home/deano/local_prefab.yaml")
    
    val fetchComponent = projectConfig.components.find(_.`type` == SourceType.Fetch).get
    fetchComponent.url shouldBe Some("https://gagameos.com/gagameos_base_prefab.yaml")
    
    val gitComponent = projectConfig.components.find(_.`type` == SourceType.Git).get
    gitComponent.url shouldBe Some("https://github.com/DeanoC/bl616.git")
    
    // Verify catalogs
    projectConfig.catalogs should have size 3
    projectConfig.catalogs(0).`type` shouldBe SourceType.Git
    projectConfig.catalogs(0).url shouldBe Some("https://github.com/example/hardware-catalog.git")
    projectConfig.catalogs(1).`type` shouldBe SourceType.Local
    projectConfig.catalogs(1).path shouldBe Some("/path/to/local/catalog")
    projectConfig.catalogs(2).`type` shouldBe SourceType.Fetch
    projectConfig.catalogs(2).url shouldBe Some("https://example.com/api/catalog")
    
    // Verify defaults
    projectConfig.defaults should have size 1
    projectConfig.defaults.get("target_frequency") shouldBe Some("100MHz")
    
    // Verify definitions
    projectConfig.definitions should have size 3
    
    // Check CPU definition
    val cpuDefinition = projectConfig.definitions.find(_.name == "Cortex-A53").get
    cpuDefinition match {
      case cpu: CpuDefinitionConfig =>
        cpu.`type` shouldBe "cpu.arm.a53"
        cpu.core_count shouldBe 4
        cpu.triple shouldBe "aarch64-none-elf"
      case _ => fail("Expected CPU definition to be of type CpuDefinitionConfig")
    }
    
    // Check RAM definition
    val ramDefinition = projectConfig.definitions.find(_.name == "MainMemory").get
    ramDefinition match {
      case ram: RamDefinitionConfig =>
        ram.`type` shouldBe "ram.sram.main"
        ram.ranges should not be empty
      case _ => fail("Expected RAM definition to be of type RamDefinitionConfig")
    }
    
    /* TODO:
    // Check clock definition
    val clockDefinition = projectConfig.definitions.find(_.name == "SystemClock").get
    clockDefinition match {
      case clock: ClockDefinitionConfig =>
        clock.`type` shouldBe "clock.system.100mhz"
        clock.frequency shouldBe "100MHz"
      case _ => fail("Expected Clock definition to be of type ClockDefinitionConfig")
    }
    */
    // Verify instances
    projectConfig.instances should have size 7
    
    // Check CPU instances
    val cpu0Instance = projectConfig.instances.find(_.name == "cpu0").get
    cpu0Instance.`type` shouldBe "cpu.riscv.test"
    cpu0Instance.attributes.get("core_count").map(_.toString) shouldBe Some("4")
    cpu0Instance.attributes.get("triple").map(_.toString) shouldBe Some("riscv64-unknown-elf")
    
    val cpu1Instance = projectConfig.instances.find(_.name == "cpu1").get
    cpu1Instance.`type` shouldBe "cpu.arm.test"
    cpu1Instance.attributes.get("core_count").map(_.toString) shouldBe Some("2")
    cpu1Instance.attributes.get("triple").map(_.toString) shouldBe Some("aarch64-none-elf")
    
    // Check memory instances
    val mainMemoryInstance = projectConfig.instances.find(_.name == "main_memory").get
    mainMemoryInstance.`type` shouldBe "ram.ddr4.test"
    
    val sramInstance = projectConfig.instances.find(_.name == "sram0").get
    sramInstance.`type` shouldBe "ram.sram.test"
    
    // Check clock instance
    val clockInstance = projectConfig.instances.find(_.name == "system_clock").get
    clockInstance.`type` shouldBe "clock.test.100mhz"
    clockInstance.attributes.get("frequency").map(_.toString) shouldBe Some("100MHz")
    
    // Check IO instance
    val uartInstance = projectConfig.instances.find(_.name == "uart0").get
    uartInstance.`type` shouldBe "io.uart.test"
    uartInstance.attributes.get("visible_to_software").map(_.toString) shouldBe Some("true")

    // Check component instance
    val subModuleInstance = projectConfig.instances.find(_.name == "sub_module").get
    subModuleInstance.`type` shouldBe "component.zynqps7.test"
    
    // Check connections
    projectConfig.connections should have size 8
    
    // Check bus connections
    val cpuToMemBus = projectConfig.connections.collect {
      case bus: BusConnectionConfig if bus.connection == "cpu0 -> main_memory" => bus
    }.headOption.getOrElse(fail("Bus connection from cpu0 to main_memory not found"))
    
    cpuToMemBus.bus_protocol shouldBe "axi4"
    cpuToMemBus.bus_name shouldBe "cpu0_mem_bus"
    
    val cpu1ToSramBus = projectConfig.connections.collect {
      case bus: BusConnectionConfig if bus.connection == "cpu1 -> sram0" => bus
    }.headOption.getOrElse(fail("Bus connection from cpu1 to sram0 not found"))
    
    cpu1ToSramBus.bus_protocol shouldBe "axi4-lite"
    // Check that silent option exists and is true
    cpu1ToSramBus.silent shouldBe true 
    
    // Check port connection
    val uartTxConnection = projectConfig.connections.find(_.isInstanceOf[PortConnectionConfig])
      .map(_.asInstanceOf[PortConnectionConfig])
      .find(_.connection == "uart0.tx -> board.uart_tx").get
    
    // Check port group connection
    val gpioConnection = projectConfig.connections.find(_.isInstanceOf[PortGroupConnectionConfig])
      .map(_.asInstanceOf[PortGroupConnectionConfig])
      .find(_.connection == "cpu0.gpio -> board.leds").get
    
    // Check that first_prefix option exists and has the correct value
    gpioConnection.first_prefix shouldBe Some("gpio_")
    
    // Check that excludes option exists
    gpioConnection.excludes should not be None
    // Then check the contents of the excludes list
    gpioConnection.excludes.get should contain("gpio_3")
    gpioConnection.excludes.get should contain("gpio_4")
    
    // Check clock connection
    val clockConnection = projectConfig.connections.find(_.isInstanceOf[ClockConnectionConfig])
      .map(_.asInstanceOf[ClockConnectionConfig])
      .find(_.connection == "system_clock -> cpu0.clk").get
    
    // Check logical connection
    val logicalConnection = projectConfig.connections.find(_.isInstanceOf[LogicalConnectionConfig])
      .map(_.asInstanceOf[LogicalConnectionConfig])
      .find(_.connection == "cpu0.reset -> board.reset_button").get
    
    // Check parameters connection
    val paramsConnection = projectConfig.connections.find(_.isInstanceOf[ParametersConnectionConfig])
      .map(_.asInstanceOf[ParametersConnectionConfig])
      .find(_.connection == "cpu0 -> cpu1").get
    
    paramsConnection.parameters should have size 2
    
    val cacheParam = paramsConnection.parameters.find(_.name == "cache_size").get
    // Use asNumber to convert the Json value to a string and then to an integer
    cacheParam.value.isNumber shouldBe true
    cacheParam.value.asNumber.get.toInt shouldBe Some(8192)
    // Check that type option exists and has the correct value
    cacheParam.`type` shouldBe Some("integer")
    
    val debugParam = paramsConnection.parameters.find(_.name == "debug_mode").get
    // Use asBoolean to convert Json value to a boolean
    debugParam.value.asBoolean shouldBe Some(true)
    
    // Check constant connection
    val constantConnection = projectConfig.connections.find(_.isInstanceOf[ConstantConnectionConfig])
      .map(_.asInstanceOf[ConstantConnectionConfig])
      .find(_.connection == "cpu0.vector_base").get
    
    // Check that value option exists and has the correct value
    constantConnection.value.map(_.asString) shouldBe Some(Some("0x00000000"))
  }
}