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
}
