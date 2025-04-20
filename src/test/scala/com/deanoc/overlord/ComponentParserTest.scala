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
  val mockPrefabs = mock(classOf[PrefabCatalog])
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

  "parseDefinitionCatalog with type-safe config" should "correctly parse catalog file and extract definition data" in {
    val catalogFilePath = Paths.get("src/test/resources/test_catalog.yaml")
    val catalogYamlContent = new String(Files.readAllBytes(catalogFilePath))

    // Parse the YAML content into a CatalogFileConfig using circe
    val parsedYaml = yamlParse(catalogYamlContent).getOrElse(Json.Null)
    parsedYaml should not be Json.Null

    val catalogConfig = parsedYaml.as[CatalogFileConfig].getOrElse(CatalogFileConfig())
    
    // Assert that parsing was successful and the top-level structure is as expected
    catalogConfig.defaults should not be empty
    catalogConfig.definitions should not be empty
    catalogConfig.catalogs should not be empty
    
    // Extract and assert the defaults
    catalogConfig.defaults should have size 4
    catalogConfig.defaults.get("version") shouldBe Some("1.0")
    catalogConfig.defaults.get("author") shouldBe Some("Overlord Team")
    catalogConfig.defaults.get("description") shouldBe Some("Example catalog file")
    catalogConfig.defaults.get("active") shouldBe Some("true")
    
    // Extract and assert the definitions
    catalogConfig.definitions should have size 5
    
    // Assert the content of the CPU definition
    val cpuDefinition = catalogConfig.definitions(0)
    cpuDefinition.`type` shouldBe "cpu"
    cpuDefinition.name shouldBe "Cortex-A53"
    cpuDefinition.config.get("core_count") shouldBe Some("4")
    cpuDefinition.config.get("triple") shouldBe Some("aarch64-none-elf")
    
    // Assert the content of the RAM definition
    val ramDefinition = catalogConfig.definitions(1)
    ramDefinition.`type` shouldBe "ram"
    ramDefinition.name shouldBe "MainMemory"
    
    // Assert the content of the clock definition
    val clockDefinition = catalogConfig.definitions(2)
    clockDefinition.`type` shouldBe "clock"
    clockDefinition.name shouldBe "SystemClock"
    clockDefinition.config.get("frequency") shouldBe Some("100MHz")
    
    // Assert the content of the IO definition
    val ioDefinition = catalogConfig.definitions(3)
    ioDefinition.`type` shouldBe "io"
    ioDefinition.name shouldBe "GPIO"
    ioDefinition.config.get("visible_to_software") shouldBe Some("true")
    
    // Assert the content of the pingroup definition
    val pinGroupDefinition = catalogConfig.definitions(4)
    pinGroupDefinition.`type` shouldBe "pingroup"
    pinGroupDefinition.name shouldBe "ControlPins"
    
    // Verify catalogs
    catalogConfig.catalogs should have size 3
    catalogConfig.catalogs(0).`type` shouldBe "git"
    catalogConfig.catalogs(0).url shouldBe Some("https://github.com/example/hardware-catalog.git")
    catalogConfig.catalogs(1).`type` shouldBe "local"
    catalogConfig.catalogs(1).path shouldBe Some("/path/to/local/catalog")
    catalogConfig.catalogs(2).`type` shouldBe "fetch"
    catalogConfig.catalogs(2).url shouldBe Some("https://example.com/api/catalog")
    
    val defType = DefinitionType.apply(cpuDefinition.`type`)
    defType.ident should contain("cpu")
    
    val mockDef = mock(classOf[HardwareDefinitionTrait])
    when(mockDef.defType).thenReturn(defType)
    
    mockDef.defType.ident should contain("cpu")
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
    val localComponent = projectConfig.components.find(_.`type` == "local").get
    localComponent.path shouldBe Some("/home/deano/local_prefab.yaml")
    
    val fetchComponent = projectConfig.components.find(_.`type` == "fetch").get
    fetchComponent.url shouldBe Some("https://gagameos.com/gagameos_base_prefab.yaml")
    
    val gitComponent = projectConfig.components.find(_.`type` == "git").get
    gitComponent.url shouldBe Some("https://github.com/DeanoC/bl616.git")
    
    // Verify catalogs
    projectConfig.catalogs should have size 3
    projectConfig.catalogs(0).`type` shouldBe "git"
    projectConfig.catalogs(0).url shouldBe Some("https://github.com/example/hardware-catalog.git")
    projectConfig.catalogs(1).`type` shouldBe "local"
    projectConfig.catalogs(1).path shouldBe Some("/path/to/local/catalog")
    projectConfig.catalogs(2).`type` shouldBe "fetch"
    projectConfig.catalogs(2).url shouldBe Some("https://example.com/api/catalog")
    
    // Verify defaults
    projectConfig.defaults should have size 1
    projectConfig.defaults.get("target_frequency") shouldBe Some("100MHz")
    
    // Verify instances
    projectConfig.instances should have size 7
    
    // Check CPU instances
    val cpu0Instance = projectConfig.instances.find(_.name == "cpu0").get
    cpu0Instance.`type` shouldBe "cpu.riscv.test"
    cpu0Instance.config.get("core_count") shouldBe "4"
    cpu0Instance.config.get("triple") shouldBe "riscv64-unknown-elf"
    
    val cpu1Instance = projectConfig.instances.find(_.name == "cpu1").get
    cpu1Instance.`type` shouldBe "cpu.arm.test"
    cpu1Instance.config.get("core_count") shouldBe "2"
    cpu1Instance.config.get("triple") shouldBe "aarch64-none-elf"
    
    // Check memory instances
    val mainMemoryInstance = projectConfig.instances.find(_.name == "main_memory").get
    mainMemoryInstance.`type` shouldBe "ram.ddr4.test"
    
    val sramInstance = projectConfig.instances.find(_.name == "sram0").get
    sramInstance.`type` shouldBe "ram.sram.test"
    
    // Check clock instance
    val clockInstance = projectConfig.instances.find(_.name == "system_clock").get
    clockInstance.`type` shouldBe "clock.test.100mhz"
    clockInstance.config.get("frequency") shouldBe "100MHz"
    
    // Check IO instance
    val uartInstance = projectConfig.instances.find(_.name == "uart0").get
    uartInstance.`type` shouldBe "io.uart.test"
    uartInstance.config.get("visible_to_software").toString shouldBe "true"
    
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
    cpu1ToSramBus.silent shouldBe Some(true)
    
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

  // For backward compatibility, also test the old YAML parsing method
  "parseDefinitionCatalog YAML parsing" should "correctly parse YAML content and extract definition data" in {
    val yamlContent = """
      defaults:
        version: 1
        author: "Test Author"
      definition:
        - type: hardware.chip.test_chip
          ident: test_chip_1
          description: "A test chip definition"
          parameters:
            param1: value1
        - type: software.program.test_program
          ident: test_program_1
          description: "A test program definition"
          source: src/test_program.c
    """.stripMargin

    // Simulate parsing the YAML content into a Map[String, Variant]
    val parsedYaml: Map[String, Variant] = Utils.fromYaml(yamlContent)

    // Assert that parsing was successful and the top-level structure is as expected
    parsedYaml should not be empty
    parsedYaml should contain key "defaults"
    parsedYaml should contain key "definition"

    // Extract and assert the defaults
    val defaults = parsedYaml.get("defaults") match {
      case Some(TableV(defaultsMap)) => defaultsMap
      case _ => fail("Defaults section not found or not a TableV")
    }
    defaults should have size 2
    defaults.get("version") match {
      case Some(IntV(value)) => value should be (1)
      case _ => fail("Version default not found or not an IntV")
    }
    defaults.get("author") match {
      case Some(StringV(value)) => value should be ("Test Author")
      case _ => fail("Author default not found or not a StringV")
    }

    // Extract and assert the definitions array
    val definitionsArray = parsedYaml.get("definition") match {
      case Some(ArrayV(defsArray)) => defsArray
      case _ => fail("Definition section not found or not an ArrayV")
    }
    definitionsArray should have size 2

    // Assert the content of the first definition entry (chip)
    val chipDefinitionEntry = definitionsArray(0) match {
      case TableV(entryMap) => entryMap
      case _ => fail("First definition entry is not a TableV")
    }
    chipDefinitionEntry should contain key "type"
    chipDefinitionEntry.get("type") match {
      case Some(StringV(value)) => value should be ("hardware.chip.test_chip")
      case _ => fail("Chip definition type not found or not a StringV")
    }
    chipDefinitionEntry should contain key "ident"
    chipDefinitionEntry.get("ident") match {
      case Some(StringV(value)) => value should be ("test_chip_1")
      case _ => fail("Chip definition ident not found or not a StringV")
    }
    chipDefinitionEntry should contain key "description"
    chipDefinitionEntry.get("description") match {
      case Some(StringV(value)) => value should be ("A test chip definition")
      case _ => fail("Chip definition description not found or not a StringV")
    }
    chipDefinitionEntry should contain key "parameters"
    chipDefinitionEntry.get("parameters") match {
      case Some(TableV(params)) => params.get("param1") match {
        case Some(StringV(value)) => value should be ("value1")
        case _ => fail("Chip parameter 'param1' not found or not a StringV")
      }
      case _ => fail("Parameters not found or not a TableV in chip definition entry")
    }

    // Assert the content of the second definition entry (program)
    val programDefinitionEntry = definitionsArray(1) match {
      case TableV(entryMap) => entryMap
      case _ => fail("Second definition entry is not a TableV")
    }
    programDefinitionEntry should contain key "type"
    programDefinitionEntry.get("type") match {
      case Some(StringV(value)) => value should be ("software.program.test_program")
      case _ => fail("Program definition type not found or not a StringV")
    }
    programDefinitionEntry should contain key "ident"
    programDefinitionEntry.get("ident") match {
      case Some(StringV(value)) => value should be ("test_program_1")
      case _ => fail("Program definition ident not found or not a StringV")
    }
    programDefinitionEntry should contain key "description"
    programDefinitionEntry.get("description") match {
      case Some(StringV(value)) => value should be ("A test program definition")
      case _ => fail("Program definition description not found or not a StringV")
    }
    programDefinitionEntry should contain key "source"
    programDefinitionEntry.get("source") match {
      case Some(StringV(value)) => value should be ("src/test_program.c")
      case _ => fail("Program source not found or not a StringV")
    }
  }
}
