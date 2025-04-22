package com.deanoc.overlord

import com.deanoc.overlord.{CatalogLoader, ComponentParser}
import com.deanoc.overlord.definitions.{DefinitionType, DefinitionTrait, SoftwareDefinitionTrait, Definition, HardwareDefinition}

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
import io.circe.{Json, Decoder, DecodingFailure}

import java.nio.file.{Files, Path, Paths}
import java.io.{File, FileWriter}
import scala.collection.mutable

class ComponentAdvancedTest
    extends AnyFlatSpec
    with Matchers
    with BeforeAndAfter
    with BeforeAndAfterAll {

  // Create temp directory for test files - shared across all tests
  var tempDir: Path = _

  override def beforeAll(): Unit = {
    // Create temp directory once before all tests
    tempDir = Files.createTempDirectory("overlord_advanced_test_")
  }

  "Component" should "load flat_serv_project.yaml correctly" in {
    // Use Java resources stream to load the file as it will be packaged in the JAR
    val resourceStream = getClass.getClassLoader.getResourceAsStream("flat_serv_project.yaml")
    val sourceFilePath = tempDir.resolve("flat_serv_project.yaml")
    Files.copy(resourceStream, sourceFilePath, java.nio.file.StandardCopyOption.REPLACE_EXISTING)

    // Use Component.fromTopLevelComponentFile to load it
    try {
      val component = Component.fromTopLevelComponentFile("TestGame", "TestBoard", sourceFilePath)
    } catch {
      case e: DecodingFailure =>
        // If a decoding failure occurs, the test fails
        e.printStackTrace()
        fail("Test failed: flat_serv_project.yaml not loaded correctly")
      case e: Exception =>
        // If an exception is thrown, the test fails
        e.printStackTrace()
        fail("Test failed: flat_serv_project.yaml not loaded correctly")
    }
  }

  "Component" should "load git_serv_project.yaml correctly" in {
    val resourceStream = getClass.getClassLoader.getResourceAsStream("git_serv_project.yaml")
    val sourceFilePath = tempDir.resolve("git_serv_project.yaml")
    Files.copy(resourceStream, sourceFilePath, java.nio.file.StandardCopyOption.REPLACE_EXISTING)

    // Use Component.fromTopLevelComponentFile to load it
    try {
      val component = Component.fromTopLevelComponentFile("TestGame", "TestBoard", sourceFilePath)
    } catch {
      case e: DecodingFailure =>
        // If a decoding failure occurs, the test fails
        e.printStackTrace()
        fail("Test failed: git_serv_project.yaml not loaded correctly")
      case e: Exception =>
        // If an exception is thrown, the test fails
        e.printStackTrace()
        fail("Test failed: git_serv_project.yaml not loaded correctly")
    }
  }  
}
