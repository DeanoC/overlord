package com.deanoc.overlord

import com.deanoc.overlord.{CatalogLoader, ComponentParser, Definition, DefinitionType, DefinitionTrait, HardwareDefinitionTrait, SoftwareDefinitionTrait}

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

  val catalogYamlContent = """
  # Default values applicable to all definitions
  defaults:
    active: true
    version: "1.0"
    author: "Overlord Team"
    description: "Example catalog file"
  
  # External catalog sources
  catalogs:
    - type: "git"
      url: "https://github.com/example/hardware-catalog.git"
    - type: "local"
      path: "/path/to/local/catalog"
    - type: "fetch"
      url: "https://example.com/api/catalog"
  
  # Hardware component definitions
  definitions:
    - name: "Cortex-A53"
      type: "cpu"
      config:
        core_count: 4
        triple: "aarch64-none-elf"
    
    - name: "MainMemory"
      type: "ram"
      config:
        ranges:
          - address: "0x80000000"
            size: "0x40000000"
          - address: "0xC0000000" 
            size: "0x20000000"
    
    - name: "SystemClock"
      type: "clock"
      config:
        frequency: "100MHz"
    
    - name: "GPIO"
      type: "io"
      config:
        visible_to_software: true
    
    - name: "ControlPins"
      type: "pingroup"
      config:
        pins: ["reset", "boot", "power"]
        direction: "output"
      """.stripMargin


  val projectYamlContent = """
# List of board names or identifiers
boards:
  - "arty_a7_35t"
  - "zcu102"

# Default settings for the project
defaults:
  project_version: "1.0.0"
  author: "Overlord Development Team"
  description: "Example SoC design with RISC-V cores"
  target_frequency: "100MHz"

# Hardware and software instances
instances:
  # CPU instances
  - name: "cpu0"
    type: "hardware.cpu.riscv"
    config:
      core_count: 4
      triple: "riscv64-unknown-elf"
  
  - name: "cpu1"
    type: "hardware.cpu.arm"
    config:
      core_count: 2
      triple: "aarch64-none-elf"
  
  # Memory instances
  - name: "main_memory"
    type: "hardware.ram.ddr4"
    config:
      ranges:
        - address: "0x80000000"
          size: "0x40000000"
  
  - name: "sram0"
    type: "hardware.ram.sram"
    config:
      ranges:
        - address: "0x00100000"
          size: "0x00010000"
  
  # Clock instance
  - name: "system_clock"
    type: "hardware.clock"
    config:
      frequency: "100MHz"
  
  # IO instance
  - name: "uart0"
    type: "hardware.io.uart"
    config:
      visible_to_software: true

# Connections between instances
connections:
  # Bus connections
  - type: "bus"
    connection: "cpu0 -> main_memory"
    bus_protocol: "axi4"
    bus_name: "cpu0_mem_bus"
  
  - type: "bus"
    connection: "cpu1 -> sram0"
    bus_protocol: "axi4-lite"
    silent: true
  
  # Port connections
  - type: "port"
    connection: "uart0.tx -> board.uart_tx"
  
  # Port group connections
  - type: "port_group"
    connection: "cpu0.gpio -> board.leds"
    first_prefix: "gpio_"
    excludes: ["gpio_3", "gpio_4"]
  
  # Clock connections
  - type: "clock"
    connection: "system_clock -> cpu0.clk"
  
  # Logical connections
  - type: "logical"
    connection: "cpu0.reset -> board.reset_button"
  
  # Parameters connections
  - type: "parameters"
    connection: "cpu0 -> cpu1"
    parameters:
      - name: "cache_size"
        value: 8192
        type: "integer"
      - name: "debug_mode"
        value: true
  
  # Constant connections
  - type: "constant"
    connection: "cpu0.vector_base"
    value: "0x00000000"

# Prefabs to include
prefabs:
  - name: "networking_stack"
  - name: "graphics_subsystem"
  - name: "audio_interface"
  """.stripMargin

  // Helper method to create a test YAML file
  def createYamlFile(
      content: String,
      fileName: String
  ): Path = {
    val filePath = tempDir.resolve(fileName)
    val writer = new FileWriter(filePath.toFile)
    writer.write(content)
    writer.close()
    filePath
  }

  override def beforeAll(): Unit = {
    // Create temp directory once before all tests
    tempDir = Files.createTempDirectory("overlord_test_")
    // create a valid catalog file for testing
    createYamlFile(catalogYamlContent, "test_catalog.yaml")
    createYamlFile(projectYamlContent, "test_project.yaml")
  }

  override def afterAll(): Unit = {
    // Clean up temp directory once after all tests are done
    deleteRecursively(tempDir.toFile)
  }

  before {
    // Create a fresh parser before each test
    parser = new ComponentParser()

    Overlord.resetPaths()
    // Add a default catalog path for testing
    Overlord.pushCatalogPath(tempDir)
  }

  // Helper method to delete directory recursively
  def deleteRecursively(file: File): Unit = {
    if (file.isDirectory) {
      file.listFiles.foreach(deleteRecursively)
    }
    file.delete()
  }


  "parseDefinitionCatalog with type-safe config" should "correctly parse catalog file and extract definition data" in {
   
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
    
    // The remaining code can stay the same or be updated based on your requirements
    val defType = DefinitionType.apply(cpuDefinition.`type`)
    defType.ident should contain("cpu")
    
    // Create a mock definition using the configuration
    val mockDef = mock(classOf[HardwareDefinitionTrait])
    when(mockDef.defType).thenReturn(defType)
    
    // Verify the mock definition
    mockDef.defType.ident should contain("cpu")
  }
  
  "parseProjectFile with type-safe config" should "correctly parse project file and extract instance data" in {
        
    // Parse the YAML content into a ProjectFileConfig using circe
    val parsedYaml = yamlParse(projectYamlContent).getOrElse(Json.Null)
    parsedYaml should not be Json.Null

    val projectConfig = parsedYaml.as[ProjectFileConfig].getOrElse(ProjectFileConfig(info = InfoConfig()))
    
    // Assert that parsing was successful and the top-level structure is as expected
    projectConfig.instances should not be empty
    projectConfig.connections should not be empty
    projectConfig.boards should not be empty
    projectConfig.prefabs should not be empty
    projectConfig.defaults should not be empty
    
    // Extract and assert the boards
    projectConfig.boards should have size 2
    projectConfig.boards should contain ("arty_a7_35t")
    projectConfig.boards should contain ("zcu102")
    
    // Extract and assert the defaults
    projectConfig.defaults should have size 4
    projectConfig.defaults.get("project_version") shouldBe Some("1.0.0")
    projectConfig.defaults.get("author") shouldBe Some("Overlord Development Team")
    
    // Extract and assert the instances
    projectConfig.instances should have size 6
    
    // Assert the content of the first instance (CPU0)
    val cpuInstance = projectConfig.instances(0)
    cpuInstance.name shouldBe "cpu0"
    cpuInstance.`type` shouldBe "hardware.cpu.riscv"
    cpuInstance.config should not be empty
    
    // Extract the CPU config from the generic config map
    val cpuConfigMap = cpuInstance.config.get
    cpuConfigMap.get("core_count") shouldBe Some("4")
    cpuConfigMap.get("triple") shouldBe Some("riscv64-unknown-elf")
    
    // Create a type-safe CpuConfig from the generic config
    val cpuConfig = CpuConfig(
      core_count = cpuConfigMap.get("core_count").map(_.toString.toInt).getOrElse(1),
      triple = cpuConfigMap.get("triple").map(_.toString).getOrElse("")
    )
    
    // Verify the type-safe config
    cpuConfig.core_count shouldBe 4
    cpuConfig.triple shouldBe "riscv64-unknown-elf"
    
    // Assert the content of the memory instance
    val ramInstance = projectConfig.instances(2)
    ramInstance.name shouldBe "main_memory"
    ramInstance.`type` shouldBe "hardware.ram.ddr4"
    ramInstance.config should not be empty
    
    // Assert the content of the IO instance
    val ioInstance = projectConfig.instances(5)
    ioInstance.name shouldBe "uart0" 
    ioInstance.`type` shouldBe "hardware.io.uart"
    ioInstance.config.get.get("visible_to_software") shouldBe Some("true")
    
    // Extract and assert the connections
    projectConfig.connections should have size 8
    
    // Assert the content of the first bus connection
    val busConnection = projectConfig.connections(0).asInstanceOf[BusConnectionConfig]
    busConnection.`type` shouldBe "bus"
    busConnection.connection shouldBe "cpu0 -> main_memory"
    busConnection.bus_protocol shouldBe Some("axi4")
    busConnection.bus_name shouldBe Some("cpu0_mem_bus")
    
    // Assert the content of the port connection
    val portConnection = projectConfig.connections(2).asInstanceOf[PortConnectionConfig]
    portConnection.`type` shouldBe "port"
    portConnection.connection shouldBe "uart0.tx -> board.uart_tx"
    
    // Assert the content of the parameters connection
    val paramsConnection = projectConfig.connections(6).asInstanceOf[ParametersConnectionConfig]
    paramsConnection.`type` shouldBe "parameters"
    paramsConnection.connection shouldBe "cpu0 -> cpu1"
    paramsConnection.parameters should have size 2
    paramsConnection.parameters(0).name shouldBe "cache_size"
    
    // Assert the prefabs
    projectConfig.prefabs should have size 3
    projectConfig.prefabs(0).name shouldBe "networking_stack"
    projectConfig.prefabs(1).name shouldBe "graphics_subsystem"
    projectConfig.prefabs(2).name shouldBe "audio_interface"
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
