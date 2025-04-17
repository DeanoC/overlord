package com.deanoc.overlord.templates

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.scalatest.BeforeAndAfterEach
import org.scalatest.BeforeAndAfterAll

import java.nio.file.{Files, Path, Paths}
import java.io.File
import scala.util.{Try, Success, Failure}
import scala.jdk.CollectionConverters._

/** Test suite for the TemplateManager object.
  *
  * This test suite focuses on the downloadStandardTemplates functionality to
  * ensure templates are downloaded correctly and are valid git repositories.
  */
class TemplateManagerSpec
    extends AnyFlatSpec
    with Matchers
    with BeforeAndAfterEach
    with BeforeAndAfterAll {

  // Temporary directory for testing
  private var tempDir: Path = _

  override def beforeAll(): Unit = {
    super.beforeAll()
    // Create a temporary directory for testing
    tempDir = Files.createTempDirectory("template-test")
  }

  override def afterAll(): Unit = {
    // Clean up after tests by deleting the temporary directory
    deleteDirectory(tempDir.toFile)
    super.afterAll()
  }

  override def beforeEach(): Unit = {
    super.beforeEach()
    // Set the template base path to our temporary directory
    TemplateManager.setTemplateBasePath(tempDir)
  }

  /** Helper method to recursively delete a directory
    */
  private def deleteDirectory(directory: File): Boolean = {
    if (directory.isDirectory) {
      val children = directory.listFiles()
      if (children != null) {
        children.foreach(deleteDirectory)
      }
    }
    directory.delete()
  }

  /** Helper method to check if a directory is a valid git repository
    */
  private def isValidGitRepository(directory: Path): Boolean = {
    val gitDir = directory.resolve(".git")

    // Check if .git directory exists
    if (!Files.isDirectory(gitDir)) {
      return false
    }

    // Check for essential git files
    val essentialGitFiles = List(
      gitDir.resolve("HEAD"),
      gitDir.resolve("config"),
      gitDir.resolve("refs")
    )

    essentialGitFiles.forall(Files.exists(_))
  }

  "TemplateManager.downloadStandardTemplates" should "download all templates and create valid git repositories" in {
    // Get the templates that should be downloaded
    val templates = TemplateManager.getStandardTemplates
    templates should not be empty

    // Download the templates
    val result = TemplateManager.downloadStandardTemplates(autoYes = true)
    result shouldBe true

    // Check that all expected template directories were created
    val templateNames = templates.map(_._1)
    val availableTemplates = TemplateManager.listAvailableTemplates()

    // All expected templates should be available
    templateNames.foreach { name =>
      availableTemplates should contain(name)
    }

    // All downloaded templates should be valid git repositories
    templateNames.foreach { name =>
      val templatePath = tempDir.resolve(name)
      Files.isDirectory(templatePath) shouldBe true
      isValidGitRepository(templatePath) shouldBe true
    }
  }

  it should "handle invalid repositories gracefully" in {
    // Save the original templates
    val originalTemplates = TemplateManager.getStandardTemplates

    try {
      // Set an invalid template
      val invalidTemplates = List(("invalid-template", "invalid/repo"))
      TemplateManager.setStandardTemplates(invalidTemplates)

      // Try to download the invalid template
      val result = TemplateManager.downloadStandardTemplates(autoYes = true)
      result shouldBe false

      // Verify no templates were downloaded
      val availableTemplates = TemplateManager.listAvailableTemplates()
      availableTemplates should not contain "invalid-template"
    } finally {
      // Restore the original templates
      TemplateManager.setStandardTemplates(originalTemplates)
    }
  }
}
