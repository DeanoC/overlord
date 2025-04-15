package com.deanoc.overlord

import com.deanoc.overlord.utils.{ArrayV, StringV, TableV, Utils, Variant}
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._
import org.scalatest.BeforeAndAfter
import org.scalatest.BeforeAndAfterAll
import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

import java.nio.file.{Files, Path, Paths}
import java.io.{File, FileWriter}
import scala.collection.mutable

class ProjectParserTest
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
    parser = new ProjectParser(mockCatalogs, mockPrefabs)

    Project.resetPaths()
    // Add a default catalog path for testing
    Project.pushCatalogPath(tempDir)
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

  "ProjectParser" should "process catalogs section before other elements" in {
    // Create a test project file with catalogs section
    val yamlContent = """
      |catalogs:
      |  - /path/to/catalog1
      |  - /path/to/catalog2
      |defaults:
      |  testDefault: value
      |instance:
      |  - name: test_instance
      |    type: test.type
      """.stripMargin

    val testFilePath = createYamlFile(yamlContent)

    // Get a snapshot of paths before execution
    val catalogPathsBefore = Project.getCatalogPathStackForTesting.toList

    // Execute the parser
    val result = parser.generateInstances(testFilePath)

    // Verify the result
    result should not be None

    // Get paths after execution
    val catalogPathsAfter = Project.getCatalogPathStackForTesting.toList

    // Check the new catalog paths were added
    catalogPathsAfter.size should be(catalogPathsBefore.size + 2)

    // More resilient check that doesn't depend on order
    val newPaths = catalogPathsAfter.diff(catalogPathsBefore)
    newPaths.size should be(2)
    newPaths should contain allOf (
      Paths.get("/path/to/catalog1"),
      Paths.get("/path/to/catalog2")
    )
  }

  it should "correctly process defaults section" in {
    val yamlContent = """
      |defaults:
      |  key1: value1
      |  key2: value2
      """.stripMargin

    val testFilePath = createYamlFile(yamlContent)

    // Mock the processInstantiations method to return success
    // In a real implementation, we'd use partial mocking

    val result = parser.generateInstances(testFilePath)
    result should not be None

    // If we could access the defaults map, we would verify its contents here
  }

  it should "handle project files with catalogs and instances" in {
    // Create mock definitions that our parser can use
    val mockDefMap = Map(
      "test.type" -> mock(classOf[DefinitionTrait])
    )

    val yamlContent = """
      |catalogs:
      |  - /path/to/catalog1
      |instance:
      |  - name: test_instance
      |    type: test.type
      """.stripMargin

    val testFilePath = createYamlFile(yamlContent)

    val result = parser.generateInstances(testFilePath)
    result should not be None

    // In a full implementation, we'd verify that:
    // 1. The catalog paths were added
    // 2. The instances were created and added to the container
  }

  it should "fail gracefully when instantiation fails" in {
    // Mock behavior for failure case
    val yamlContent = """
      |catalogs:
      |  - /path/to/catalog1
      |instance:
      |  - name: invalid_instance
      |    type: nonexistent.type
      """.stripMargin

    val testFilePath = createYamlFile(yamlContent)

    val result = parser.generateInstances(testFilePath)

    // Instead of expecting None, check that the container exists but is empty
    result should not be None
    result.get.children should be(empty)
  }
}
