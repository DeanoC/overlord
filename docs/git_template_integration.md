# Git/GitHub Integration for Templates

This document outlines how git and GitHub functionality can be integrated into the template system for Overlord.

## Overview

By integrating git/GitHub functionality into the template system, we can enable:

1. Versioning of templates
2. Sharing templates via GitHub repositories
3. Updating templates from remote sources
4. Collaborative template development

## Implementation Plan

### 1. Template Sources

Templates can come from multiple sources:

1. **Local templates**: Stored in `~/.overlord/templates/`
2. **Git repositories**: Cloned from GitHub or other git hosts
3. **GitHub templates**: Directly downloaded from GitHub using the API

### 2. Template Registry

Create a template registry to track template sources:

```
~/.overlord/template-registry.json
```

Example registry content:

```json
{
  "templates": [
    {
      "name": "bare-metal",
      "type": "local",
      "path": "~/.overlord/templates/bare-metal"
    },
    {
      "name": "linux-app",
      "type": "git",
      "url": "https://github.com/example/overlord-linux-template.git",
      "branch": "main",
      "path": "~/.overlord/templates/linux-app"
    },
    {
      "name": "fpga-basic",
      "type": "github",
      "owner": "example",
      "repo": "overlord-fpga-template",
      "ref": "v1.0.0",
      "path": "~/.overlord/templates/fpga-basic"
    }
  ]
}
```

### 3. Template Commands

Add commands for managing templates with git integration:

```
overlord template list                                # List all templates
overlord template add <name> <path>                   # Add a local template
overlord template add-git <name> <git-url> [branch]   # Add a template from git
overlord template add-github <name> <owner/repo> [ref] # Add a template from GitHub
overlord template remove <name>                       # Remove a template
overlord template update <name>                       # Update a template from its source
overlord template update-all                          # Update all templates
```

### 4. Template Manager Implementation

Extend the `TemplateManager` to handle git operations:

```scala
object TemplateManager {
  private val templateBasePath = Paths.get(System.getProperty("user.home"), ".overlord", "templates")
  private val registryPath = Paths.get(System.getProperty("user.home"), ".overlord", "template-registry.json")

  case class TemplateSource(
    name: String,
    sourceType: String, // "local", "git", or "github"
    path: String,
    url: Option[String] = None,
    branch: Option[String] = None,
    owner: Option[String] = None,
    repo: Option[String] = None,
    ref: Option[String] = None
  )

  case class TemplateRegistry(templates: List[TemplateSource])

  // Load the template registry
  def loadRegistry(): TemplateRegistry = {
    if (!Files.exists(registryPath)) {
      TemplateRegistry(List.empty)
    } else {
      // Parse JSON registry file
      // ...
    }
  }

  // Save the template registry
  def saveRegistry(registry: TemplateRegistry): Unit = {
    // Convert registry to JSON and save
    // ...
  }

  // List all templates
  def listTemplates(): List[TemplateSource] = {
    loadRegistry().templates
  }

  // Add a local template
  def addLocalTemplate(name: String, path: String): Boolean = {
    val sourcePath = Paths.get(path)
    if (!Files.exists(sourcePath)) {
      error(s"Template source path '$path' does not exist")
      return false
    }

    val registry = loadRegistry()
    if (registry.templates.exists(_.name == name)) {
      error(s"Template with name '$name' already exists")
      return false
    }

    val targetPath = templateBasePath.resolve(name)
    if (Files.exists(targetPath)) {
      error(s"Target path '$targetPath' already exists")
      return false
    }

    // Copy template files
    copyDirectory(sourcePath, targetPath)

    // Update registry
    val newTemplate = TemplateSource(
      name = name,
      sourceType = "local",
      path = targetPath.toString
    )

    saveRegistry(TemplateRegistry(registry.templates :+ newTemplate))

    info(s"Added local template '$name'")
    true
  }

  // Add a template from git
  def addGitTemplate(name: String, url: String, branch: Option[String] = None): Boolean = {
    val registry = loadRegistry()
    if (registry.templates.exists(_.name == name)) {
      error(s"Template with name '$name' already exists")
      return false
    }

    val targetPath = templateBasePath.resolve(name)
    if (Files.exists(targetPath)) {
      error(s"Target path '$targetPath' already exists")
      return false
    }

    // Clone git repository
    val branchArg = branch.map(b => s"-b $b").getOrElse("")
    val cloneCommand = s"git clone $branchArg $url $targetPath"

    val cloneResult = cloneCommand.!
    if (cloneResult != 0) {
      error(s"Failed to clone git repository from '$url'")
      return false
    }

    // Update registry
    val newTemplate = TemplateSource(
      name = name,
      sourceType = "git",
      path = targetPath.toString,
      url = Some(url),
      branch = branch
    )

    saveRegistry(TemplateRegistry(registry.templates :+ newTemplate))

    info(s"Added git template '$name' from '$url'")
    true
  }

  // Add a template from GitHub
  def addGitHubTemplate(name: String, ownerRepo: String, ref: Option[String] = None): Boolean = {
    val parts = ownerRepo.split("/")
    if (parts.length != 2) {
      error(s"Invalid GitHub repository format. Expected 'owner/repo', got '$ownerRepo'")
      return false
    }

    val owner = parts(0)
    val repo = parts(1)

    val registry = loadRegistry()
    if (registry.templates.exists(_.name == name)) {
      error(s"Template with name '$name' already exists")
      return false
    }

    val targetPath = templateBasePath.resolve(name)
    if (Files.exists(targetPath)) {
      error(s"Target path '$targetPath' already exists")
      return false
    }

    // Download from GitHub
    val refParam = ref.getOrElse("main")
    val url = s"https://github.com/$owner/$repo/archive/$refParam.zip"

    // Create temporary directory for download
    val tempDir = Files.createTempDirectory("overlord-template")
    val zipFile = tempDir.resolve(s"$repo.zip")

    // Download zip file
    val downloadCommand = s"curl -L $url -o $zipFile"
    val downloadResult = downloadCommand.!

    if (downloadResult != 0) {
      error(s"Failed to download template from GitHub: $url")
      return false
    }

    // Extract zip file
    val extractCommand = s"unzip -q $zipFile -d $tempDir"
    val extractResult = extractCommand.!

    if (extractResult != 0) {
      error(s"Failed to extract template zip file")
      return false
    }

    // Find the extracted directory
    import scala.jdk.CollectionConverters._
    val extractedDir = Files.list(tempDir)
      .iterator()
      .asScala
      .filter(Files.isDirectory(_))
      .filter(_.getFileName.toString.startsWith(s"$repo-"))
      .toList
      .headOption

    if (extractedDir.isEmpty) {
      error(s"Could not find extracted template directory")
      return false
    }

    // Copy to target path
    Files.createDirectories(targetPath.getParent)
    copyDirectory(extractedDir.get, targetPath)

    // Clean up temporary directory
    deleteDirectory(tempDir)

    // Update registry
    val newTemplate = TemplateSource(
      name = name,
      sourceType = "github",
      path = targetPath.toString,
      owner = Some(owner),
      repo = Some(repo),
      ref = ref
    )

    saveRegistry(TemplateRegistry(registry.templates :+ newTemplate))

    info(s"Added GitHub template '$name' from '$ownerRepo'")
    true
  }

  // Remove a template
  def removeTemplate(name: String): Boolean = {
    val registry = loadRegistry()
    registry.templates.find(_.name == name) match {
      case Some(template) =>
        // Delete template directory
        val templatePath = Paths.get(template.path)
        if (Files.exists(templatePath)) {
          deleteDirectory(templatePath)
        }

        // Update registry
        val updatedTemplates = registry.templates.filterNot(_.name == name)
        saveRegistry(TemplateRegistry(updatedTemplates))

        info(s"Removed template '$name'")
        true

      case None =>
        error(s"Template '$name' not found")
        false
    }
  }

  // Update a template from its source
  def updateTemplate(name: String): Boolean = {
    val registry = loadRegistry()
    registry.templates.find(_.name == name) match {
      case Some(template) =>
        template.sourceType match {
          case "local" =>
            info(s"Template '$name' is local, nothing to update")
            true

          case "git" =>
            val templatePath = Paths.get(template.path)
            if (!Files.exists(templatePath)) {
              error(s"Template directory '$templatePath' does not exist")
              return false
            }

            // Pull latest changes
            val pullCommand = Process(
              Seq("git", "pull"),
              new java.io.File(templatePath.toString)
            )

            val pullResult = pullCommand.!
            if (pullResult != 0) {
              error(s"Failed to update git template '$name'")
              return false
            }

            info(s"Updated git template '$name'")
            true

          case "github" =>
            // Re-download from GitHub
            val owner = template.owner.getOrElse("")
            val repo = template.repo.getOrElse("")
            val ref = template.ref

            // Remove existing template
            removeTemplate(name)

            // Re-add from GitHub
            addGitHubTemplate(name, s"$owner/$repo", ref)

          case _ =>
            error(s"Unknown template source type: ${template.sourceType}")
            false
        }

      case None =>
        error(s"Template '$name' not found")
        false
    }
  }

  // Update all templates
  def updateAllTemplates(): Boolean = {
    val registry = loadRegistry()
    var success = true

    registry.templates.foreach { template =>
      if (template.sourceType != "local") {
        val result = updateTemplate(template.name)
        if (!result) {
          warn(s"Failed to update template '${template.name}'")
          success = false
        }
      }
    }

    success
  }

  // Helper methods (same as before)
  private def copyDirectory(source: Path, target: Path): Unit = {
    // Implementation as before
  }

  private def deleteDirectory(path: Path): Unit = {
    // Implementation as before
  }
}
```

### 5. Command Line Interface

Update the CLI to include template management commands:

```scala
// Template command and subcommands
val templateCommand = cmd("template")
  .action((_, c) => c.copy(command = Some("template")))
  .text("Manage templates")
  .children(
    // template list subcommand
    cmd("list")
      .action((_, c) => c.copy(subCommand = Some("list")))
      .text("List all available templates"),

    // template add subcommand
    cmd("add")
      .action((_, c) => c.copy(subCommand = Some("add")))
      .text("Add a local template")
      .children(
        arg[String]("<name>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("name" -> x)))
          .text("name of the template"),

        arg[String]("<path>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("path" -> x)))
          .text("path to the template directory")
      ),

    // template add-git subcommand
    cmd("add-git")
      .action((_, c) => c.copy(subCommand = Some("add-git")))
      .text("Add a template from a git repository")
      .children(
        arg[String]("<name>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("name" -> x)))
          .text("name of the template"),

        arg[String]("<git-url>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("git-url" -> x)))
          .text("URL of the git repository"),

        opt[String]("branch")
          .action((x, c) => c.copy(options = c.options + ("branch" -> x)))
          .text("branch to use (default: main)")
      ),

    // template add-github subcommand
    cmd("add-github")
      .action((_, c) => c.copy(subCommand = Some("add-github")))
      .text("Add a template from GitHub")
      .children(
        arg[String]("<name>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("name" -> x)))
          .text("name of the template"),

        arg[String]("<owner/repo>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("owner-repo" -> x)))
          .text("GitHub repository in the format 'owner/repo'"),

        opt[String]("ref")
          .action((x, c) => c.copy(options = c.options + ("ref" -> x)))
          .text("reference to use (tag, branch, or commit hash, default: main)")
      ),

    // template remove subcommand
    cmd("remove")
      .action((_, c) => c.copy(subCommand = Some("remove")))
      .text("Remove a template")
      .children(
        arg[String]("<name>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("name" -> x)))
          .text("name of the template to remove")
      ),

    // template update subcommand
    cmd("update")
      .action((_, c) => c.copy(subCommand = Some("update")))
      .text("Update a template from its source")
      .children(
        arg[String]("<name>")
          .required()
          .action((x, c) => c.copy(options = c.options + ("name" -> x)))
          .text("name of the template to update")
      ),

    // template update-all subcommand
    cmd("update-all")
      .action((_, c) => c.copy(subCommand = Some("update-all")))
      .text("Update all templates from their sources")
  )
```

### 6. Command Execution

Add handlers for the template commands:

```scala
def executeTemplateList(config: Config): Unit = {
  val templates = TemplateManager.listTemplates()

  if (templates.isEmpty) {
    info("No templates available")
    return
  }

  info("Available templates:")
  templates.foreach { template =>
    val sourceInfo = template.sourceType match {
      case "local" => "local"
      case "git" => s"git: ${template.url.getOrElse("")}"
      case "github" => s"github: ${template.owner.getOrElse("")}/${template.repo.getOrElse("")}"
      case _ => template.sourceType
    }

    info(s"  ${template.name} ($sourceInfo)")
  }
}

def executeTemplateAdd(config: Config): Unit = {
  val name = config.options.getOrElse("name", "").asInstanceOf[String]
  val path = config.options.getOrElse("path", "").asInstanceOf[String]

  if (name.isEmpty || path.isEmpty) {
    error("Missing required arguments")
    return
  }

  TemplateManager.addLocalTemplate(name, path)
}

def executeTemplateAddGit(config: Config): Unit = {
  val name = config.options.getOrElse("name", "").asInstanceOf[String]
  val gitUrl = config.options.getOrElse("git-url", "").asInstanceOf[String]
  val branch = config.options.get("branch").map(_.asInstanceOf[String])

  if (name.isEmpty || gitUrl.isEmpty) {
    error("Missing required arguments")
    return
  }

  TemplateManager.addGitTemplate(name, gitUrl, branch)
}

def executeTemplateAddGitHub(config: Config): Unit = {
  val name = config.options.getOrElse("name", "").asInstanceOf[String]
  val ownerRepo = config.options.getOrElse("owner-repo", "").asInstanceOf[String]
  val ref = config.options.get("ref").map(_.asInstanceOf[String])

  if (name.isEmpty || ownerRepo.isEmpty) {
    error("Missing required arguments")
    return
  }

  TemplateManager.addGitHubTemplate(name, ownerRepo, ref)
}

def executeTemplateRemove(config: Config): Unit = {
  val name = config.options.getOrElse("name", "").asInstanceOf[String]

  if (name.isEmpty) {
    error("Missing required arguments")
    return
  }

  TemplateManager.removeTemplate(name)
}

def executeTemplateUpdate(config: Config): Unit = {
  val name = config.options.getOrElse("name", "").asInstanceOf[String]

  if (name.isEmpty) {
    error("Missing required arguments")
    return
  }

  TemplateManager.updateTemplate(name)
}

def executeTemplateUpdateAll(config: Config): Unit = {
  TemplateManager.updateAllTemplates()
}
```

## Conclusion

By integrating git and GitHub functionality into the template system, we enable users to:

1. Share templates with others via git repositories
2. Maintain versioned templates
3. Easily update templates from their sources
4. Collaborate on template development

This approach leverages existing git workflows that developers are already familiar with, making it easy to adopt and use.
