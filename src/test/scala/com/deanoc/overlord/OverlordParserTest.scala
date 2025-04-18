package com.deanoc.overlord

import com.deanoc.overlord.{CatalogLoader, Definition, DefinitionType, DefinitionTrait, HardwareDefinitionTrait, SoftwareDefinitionTrait}

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
import io.circe.{Json, Decoder}

import java.nio.file.{Files, Path, Paths}
import java.io.{File, FileWriter}
import scala.collection.mutable

class OverlordParserTest
    extends AnyFlatSpec
    with Matchers
    with BeforeAndAfter
    with BeforeAndAfterAll {
  // Mock objects
  val mockCatalogs = mock(classOf[DefinitionCatalog])
  val mockPrefabs = mock(classOf[PrefabCatalog])
  var parser: ProjectParser = _

  // Create temp directory for test files - shared across all tests
  var tempDir: Path = _

  override def beforeAll(): Unit = {
    // Create temp directory once before all tests
    tempDir = Files.createTempDirectory("overlord_test_")
  }

  override def afterAll(): Unit = {
    // Clean up temp directory once after all tests are done
    deleteRecursively(tempDir.toFile)
  }

  before {
    // Create a fresh parser before each test
    parser = new ProjectParser()

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

  // Helper method to create a test YAML file
  def createYamlFile(
      content: String,
      fileName: String = "test_project.yaml"
  ): Path = {
    val filePath = tempDir.resolve(fileName)
    val writer = new FileWriter(filePath.toFile)
    writer.write(content)
    writer.close()
    filePath
  }

  "parseDefinitionCatalog with type-safe config" should "correctly parse catalog file and extract definition data" in {
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

    // Create a test file with the YAML content
    val filePath = createYamlFile(yamlContent, "test_catalog.yaml")
    
    // Parse the YAML content into a CatalogFileConfig using circe
    val parsedJson = parse(yamlContent).getOrElse(Json.Null)
    val catalogConfig = parsedJson.as[CatalogFileConfig].getOrElse(CatalogFileConfig())
    
    // Assert that parsing was successful and the top-level structure is as expected
    catalogConfig.defaults should not be empty
    catalogConfig.definition should not be empty
    
    // Extract and assert the defaults
    val defaults = catalogConfig.defaults.get
    defaults should have size 2
    defaults.get("version") shouldBe Some("1")
    defaults.get("author") shouldBe Some("Test Author")
    
    // Extract and assert the definitions
    val definitions = catalogConfig.definition.get
    definitions should have size 2
    
    // Assert the content of the first definition entry (chip)
    val chipDefinition = definitions(0)
    chipDefinition.`type` shouldBe "hardware.chip.test_chip"
    chipDefinition.name shouldBe "test_chip_1"
    
    // Assert the content of the second definition entry (program)
    val programDefinition = definitions(1)
    programDefinition.`type` shouldBe "software.program.test_program"
    programDefinition.name shouldBe "test_program_1"
    
    // Verify that we can create domain objects from these configurations
    val defType = DefinitionType.apply(chipDefinition.`type`)
    defType.ident should contain("test_chip")
    
    // Create a mock definition using the configuration
    val mockDef = mock(classOf[HardwareDefinitionTrait])
    when(mockDef.defType).thenReturn(defType)
    
    // Verify the mock definition
    mockDef.defType.ident should contain("test_chip")
  }
  
  "parseProjectFile with type-safe config" should "correctly parse project file and extract instance data" in {
    val yamlContent = """
      instance:
        - name: cpu1
          type: hardware.cpu.riscv
          config:
            core_count: 2
            triple: "riscv32-unknown-elf"
        - name: ram1
          type: hardware.ram.sram
          config:
            ranges:
              - address: "0x10000000"
                size: "0x1000"
      connections:
        - type: bus
          connection: "cpu1 -> ram1"
          bus_protocol: "axi"
    """.stripMargin

    // Create a test file with the YAML content
    val filePath = createYamlFile(yamlContent, "test_project.yaml")
    
    // Parse the YAML content into a ProjectFileConfig using circe
    val parsedJson = parse(yamlContent).getOrElse(Json.Null)
    val projectConfig = parsedJson.as[ProjectFileConfig].getOrElse(ProjectFileConfig())
    
    // Assert that parsing was successful and the top-level structure is as expected
    projectConfig.instance should not be empty
    projectConfig.connections should not be empty
    
    // Extract and assert the instances
    val instances = projectConfig.instance.get
    instances should have size 2
    
    // Assert the content of the first instance (CPU)
    val cpuInstance = instances(0)
    cpuInstance.name shouldBe "cpu1"
    cpuInstance.`type` shouldBe "hardware.cpu.riscv"
    cpuInstance.config should not be empty
    
    // Extract the CPU config from the generic config map
    val cpuConfigMap = cpuInstance.config.get
    cpuConfigMap.get("core_count") shouldBe Some("2")
    cpuConfigMap.get("triple") shouldBe Some("riscv32-unknown-elf")
    
    // Create a type-safe CpuConfig from the generic config
    val cpuConfig = CpuConfig(
      core_count = cpuConfigMap.get("core_count").map(_.toString.toInt).getOrElse(1),
      triple = cpuConfigMap.get("triple").map(_.toString).getOrElse("")
    )
    
    // Verify the type-safe config
    cpuConfig.core_count shouldBe 2
    cpuConfig.triple shouldBe "riscv32-unknown-elf"
    
    // Assert the content of the second instance (RAM)
    val ramInstance = instances(1)
    ramInstance.name shouldBe "ram1"
    ramInstance.`type` shouldBe "hardware.ram.sram"
    
    // Extract and assert the connections
    val connections = projectConfig.connections.get
    connections should have size 1
    
    // Assert the content of the connection
    val busConnection = connections(0).asInstanceOf[BusConnectionConfig]
    busConnection.`type` shouldBe "bus"
    busConnection.connection shouldBe "cpu1 -> ram1"
    busConnection.bus_protocol shouldBe Some("axi")
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
