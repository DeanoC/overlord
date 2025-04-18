package com.deanoc.overlord

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatestplus.mockito.MockitoSugar
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import com.deanoc.overlord.utils.{Utils, Variant, SilentLogger}
import com.deanoc.overlord.config._
import com.deanoc.overlord.instances._
import com.deanoc.overlord.connections._
import com.deanoc.overlord.hardware.HardwareDefinition
import com.deanoc.overlord.software.SoftwareDefinition
import com.deanoc.overlord.interfaces.RamLike
import io.circe.parser._
import io.circe.Json

import java.nio.file.{Files, Path, Paths}
import java.io.{File, FileWriter}

/**
 * Test suite for the ProjectParser with type-safe configuration classes.
 * 
 * This test suite demonstrates how to use the new type-safe configuration classes
 * for testing project parsing, showing the improved testability of the refactored code.
 */
class ProjectParserSpec extends AnyFlatSpec with Matchers with MockitoSugar with SilentLogger {
  
  // Create temp directory for test files
  var tempDir: Path = _
  
  // Set up before each test
  def setup(): Unit = {
    tempDir = Files.createTempDirectory("overlord_test_")
    Overlord.resetPaths()
    Overlord.pushCatalogPath(tempDir)
  }
  
  // Clean up after each test
  def cleanup(): Unit = {
    deleteRecursively(tempDir.toFile)
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
  
  "ProjectParser" should "parse a project file using type-safe configuration" in {
    setup()
    
    try {
      // Create mock definitions
      val mockCpuDef = mock[ChipDefinitionTrait]
      val mockRamDef = mock[ChipDefinitionTrait]
      val mockClockDef = mock[ChipDefinitionTrait]
      
      // Set up the mock definitions
      val cpuDefType = DefinitionType.apply("hardware.cpu.riscv")
      val ramDefType = DefinitionType.apply("hardware.ram.sram")
      val clockDefType = DefinitionType.apply("hardware.clock.oscillator")
      
      when(mockCpuDef.defType).thenReturn(cpuDefType)
      when(mockRamDef.defType).thenReturn(ramDefType)
      when(mockClockDef.defType).thenReturn(clockDefType)
      
      when(mockCpuDef.attributes).thenReturn(Map[String, Variant]())
      when(mockRamDef.attributes).thenReturn(Map[String, Variant]())
      when(mockClockDef.attributes).thenReturn(Map[String, Variant]())
      
      // Create a test project YAML file
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
          - name: clock1
            type: hardware.clock.oscillator
            config:
              frequency: "100MHz"
        connections:
          - type: bus
            connection: "cpu1 -> ram1"
            bus_protocol: "axi"
          - type: clock
            connection: "clock1 -> cpu1"
      """.stripMargin
      
      val filePath = createYamlFile(yamlContent)
      
      // Parse the project file into a ProjectFileConfig using circe
      val parsedJson = parse(yamlContent).getOrElse(Json.Null)
      val projectConfig = parsedJson.as[ProjectFileConfig].getOrElse(ProjectFileConfig())
      
      // Verify the parsed configuration
      projectConfig.instance should not be empty
      projectConfig.connections should not be empty
      
      val instances = projectConfig.instance.get
      instances should have size 3
      
      val connections = projectConfig.connections.get
      connections should have size 2
      
      // Create type-safe configurations for each instance
      val cpuConfig = CpuConfig(
        core_count = 2,
        triple = "riscv32-unknown-elf"
      )
      
      val ramConfig = RamConfig(
        ranges = List(
          MemoryRangeConfig(
            address = "0x10000000",
            size = "0x1000"
          )
        )
      )
      
      val clockConfig = ClockConfig(
        frequency = "100MHz"
      )
      
      // Create instances using the type-safe configurations
      val cpuInstance = CpuInstance("cpu1", mockCpuDef, cpuConfig).toOption.get
      val ramInstance = new RamInstance("ram1", mockRamDef, ramConfig)
      val clockInstance = new ClockInstance("clock1", mockClockDef, clockConfig)
      
      // Verify the instances
      cpuInstance.name shouldBe "cpu1"
      cpuInstance.cpuCount shouldBe 2
      cpuInstance.triple shouldBe "riscv32-unknown-elf"
      
      // Verify RAM ranges using the RamLike interface
      val ramRanges = ramInstance.getRanges
      ramRanges should have size 1
      ramRanges(0)._1 shouldBe BigInt("10000000", 16) // address
      ramRanges(0)._2 shouldBe BigInt("1000", 16) // size
      
      // Verify clock frequency
      clockInstance.name shouldBe "clock1"
      clockInstance.frequency shouldBe "100MHz"
      
      // Create connections using the type-safe configurations
      val busConnection = connections(0).asInstanceOf[BusConnectionConfig]
      val clockConnection = connections(1).asInstanceOf[ClockConnectionConfig]
      
      // Parse the connections
      val parsedBusConnection = ConnectionParser.parseConnectionConfig(busConnection)
      val parsedClockConnection = ConnectionParser.parseConnectionConfig(clockConnection)
      
      // Verify the connections
      parsedBusConnection shouldBe defined
      parsedBusConnection.get shouldBe a[UnconnectedBus]
      val bus = parsedBusConnection.get.asInstanceOf[UnconnectedBus]
      bus.firstFullName shouldBe "cpu1"
      bus.secondFullName shouldBe "ram1"
      bus.busProtocol.value shouldBe "axi"
      
      parsedClockConnection shouldBe defined
      parsedClockConnection.get shouldBe a[UnconnectedClock]
      val clock = parsedClockConnection.get.asInstanceOf[UnconnectedClock]
      clock.firstFullName shouldBe "clock1"
      clock.secondFullName shouldBe "cpu1"
      
      // Create a sequence of instances for connection
      val instanceSeq = Seq(cpuInstance, ramInstance, clockInstance)
      
      // Connect the bus
      val connectedBuses = bus.connect(instanceSeq)
      connectedBuses should not be empty
      connectedBuses.head shouldBe a[ConnectedBus]
      val connectedBus = connectedBuses.head.asInstanceOf[ConnectedBus]
      
      // Access the instance from the InstanceLoc
      connectedBus.first.get.instance shouldBe cpuInstance
      connectedBus.second.get.instance shouldBe ramInstance
      
      // Connect the clock
      val connectedClocks = clock.connect(instanceSeq)
      connectedClocks should not be empty
      // The connected clock is likely a ConnectedBetween or similar
      val connectedClock = connectedClocks.head
      connectedClock.firstFullName shouldBe "clock1"
      connectedClock.secondFullName shouldBe "cpu1"
    } finally {
      cleanup()
    }
  }
  
  it should "handle dependency injection for instance creation" in {
    setup()
    
    try {
      // Create a mock board type
      val mockBoardType = XilinxBoard("artix7", "xc7a35t")
      
      // Create a mock definition
      val mockBoardDef = mock[ChipDefinitionTrait]
      
      // Set up the mock definition
      val boardDefType = DefinitionType.apply("hardware.board.fpga")
      when(mockBoardDef.defType).thenReturn(boardDefType)
      when(mockBoardDef.attributes).thenReturn(Map[String, Variant]())
      
      // Create a test project YAML file with a board instance
      val yamlContent = """
        instance:
          - name: fpga_board
            type: hardware.board.fpga
            config:
              board_type: "arty_a7"
              clocks:
                - name: "sys_clk"
                  frequency: "100MHz"
                - name: "aux_clk"
                  frequency: "50MHz"
      """.stripMargin
      
      val filePath = createYamlFile(yamlContent)
      
      // Parse the project file into a ProjectFileConfig using circe
      val parsedJson = parse(yamlContent).getOrElse(Json.Null)
      val projectConfig = parsedJson.as[ProjectFileConfig].getOrElse(ProjectFileConfig())
      
      // Verify the parsed configuration
      projectConfig.instance should not be empty
      
      val instances = projectConfig.instance.get
      instances should have size 1
      
      val boardInstance = instances(0)
      boardInstance.name shouldBe "fpga_board"
      boardInstance.`type` shouldBe "hardware.board.fpga"
      boardInstance.config should not be empty
      
      // Extract the board config from the generic config map
      val boardConfigMap = boardInstance.config.get
      
      // Create a type-safe BoardConfig
      val boardClocks = List(
        BoardClockConfig(
          name = "sys_clk",
          frequency = "100MHz"
        ),
        BoardClockConfig(
          name = "aux_clk",
          frequency = "50MHz"
        )
      )
      
      val boardConfig = BoardConfig(
        board_type = "arty_a7",
        clocks = boardClocks
      )
      
      // Create a board instance using the type-safe configuration
      val board = new BoardInstance(
        name = "fpga_board",
        boardType = mockBoardType,
        definition = mockBoardDef
      )
      
      // Verify the board instance
      board.name shouldBe "fpga_board"
      board.boardType shouldBe mockBoardType
      
      // Since we're using a mock, we can't verify the clocks directly
      // In a real scenario, the clocks would be created by the BoardInstance.apply method
    } finally {
      cleanup()
    }
  }
}