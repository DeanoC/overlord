# CLI Implementation Details

This document provides detailed implementation guidance for modernizing the Overlord CLI interface.

## Config Class Redesign

The current `Config` class needs to be redesigned to support the hierarchical command structure:

```scala
case class Config(
  // Primary command (create, generate, clean, update)
  command: Option[String] = None,

  // Secondary command (project, from-template, test, report, svd, catalog)
  subCommand: Option[String] = None,

  // Common options
  out: String = ".",
  board: Option[String] = None,
  nostdresources: Boolean = false,
  nostdprefabs: Boolean = false,
  resources: Option[String] = None,
  yes: Boolean = false,
  noexit: Boolean = false,
  trace: Option[String] = None,
  debug: Option[String] = None,

  // Command-specific options
  infile: Option[String] = None,
  instance: Option[String] = None,
  templateName: Option[String] = None,
  projectName: Option[String] = None,
  stdresource: Option[String] = None
)
```

## Parser Implementation

Here's a detailed implementation of the parser using scopt's builder pattern:

```scala
val parser: OParser[_, Config] = {
  val builder = OParser.builder[Config]
  import builder._

  // Common options that can be used with multiple commands
  val commonOptions = Seq(
    opt[String]("out")
      .action((x, c) => c.copy(out = x))
      .text("path where generated files should be placed"),

    opt[Unit]("nostdresources")
      .action((_, c) => c.copy(nostdresources = true))
      .text("don't use the standard catalog"),

    opt[Unit]("nostdprefabs")
      .action((_, c) => c.copy(nostdprefabs = true))
      .text("don't use the standard prefabs"),

    opt[String]("resources")
      .action((x, c) => c.copy(resources = Some(x)))
      .text("use the specified path as the root of resources"),

    opt[Unit]("yes")
      .abbr("y")
      .action((_, c) => c.copy(yes = true))
      .text("automatically agree (e.g., download resource files without prompting)"),

    opt[Unit]("noexit")
      .action((_, c) => c.copy(noexit = true))
      .text("disable automatic exit on error logs"),

    opt[String]("trace")
      .action((x, c) => c.copy(trace = Some(x)))
      .text("enable trace logging for comma-separated list of modules (can use short names)"),

    opt[String]("debug")
      .action((x, c) => c.copy(debug = Some(x)))
      .text("enable debug logging for comma-separated list of modules (can use short names)"),

    opt[String]("stdresource")
      .action((x, c) => c.copy(stdresource = Some(x)))
      .text(s"specify the standard resource path {default: ${Resources.stdResourcePath()}}")
  )

  // CREATE command and subcommands
  val createCommand = cmd("create")
    .action((_, c) => c.copy(command = Some("create")))
    .text("Create a new project or from a template")
    .children(
      // create project subcommand
      cmd("project")
        .action((_, c) => c.copy(subCommand = Some("project")))
        .text("Create a new project from an .over file")
        .children(
          arg[String]("<infile>")
            .required()
            .action((x, c) => c.copy(infile = Some(x)))
            .text("filename should be a .over file to use for the project"),

          opt[String]("board")
            .required()
            .action((x, c) => c.copy(board = Some(x)))
            .text("board definition to use")
        ),

      // create from-template subcommand
      cmd("from-template")
        .action((_, c) => c.copy(subCommand = Some("from-template")))
        .text("Create a new project from a template")
        .children(
          arg[String]("<template-name>")
            .required()
            .action((x, c) => c.copy(templateName = Some(x)))
            .text("name of the template to use (e.g., bare-metal, linux-app)"),

          arg[String]("<project-name>")
            .required()
            .action((x, c) => c.copy(projectName = Some(x)))
            .text("name of the project to create")
        )
    )

  // GENERATE command and subcommands
  val generateCommand = cmd("generate")
    .action((_, c) => c.copy(command = Some("generate")))
    .text("Generate various outputs")
    .children(
      // generate test subcommand
      cmd("test")
        .action((_, c) => c.copy(subCommand = Some("test")))
        .text("Generate test files for a project")
        .children(
          arg[String]("<project-name>")
            .required()
            .action((x, c) => c.copy(projectName = Some(x)))
            .text("name of the project to generate tests for")
        ),

      // generate report subcommand
      cmd("report")
        .action((_, c) => c.copy(subCommand = Some("report")))
        .text("Generate a report about the project structure")
        .children(
          arg[String]("<infile>")
            .required()
            .action((x, c) => c.copy(infile = Some(x)))
            .text("filename should be a .over file to use for the project")
        ),

      // generate svd subcommand
      cmd("svd")
        .action((_, c) => c.copy(subCommand = Some("svd")))
        .text("Generate a CMSIS-SVD file")
        .children(
          arg[String]("<infile>")
            .required()
            .action((x, c) => c.copy(infile = Some(x)))
            .text("filename should be a .over file to use for the project")
        )
    )

  // CLEAN command and subcommands
  val cleanCommand = cmd("clean")
    .action((_, c) => c.copy(command = Some("clean")))
    .text("Clean generated files")
    .children(
      // clean test subcommand
      cmd("test")
        .action((_, c) => c.copy(subCommand = Some("test")))
        .text("Clean test files for a project")
        .children(
          arg[String]("<project-name>")
            .required()
            .action((x, c) => c.copy(projectName = Some(x)))
            .text("name of the project to clean tests for")
        )
    )

  // UPDATE command and subcommands
  val updateCommand = cmd("update")
    .action((_, c) => c.copy(command = Some("update")))
    .text("Update various components")
    .children(
      // update project subcommand
      cmd("project")
        .action((_, c) => c.copy(subCommand = Some("project")))
        .text("Update an existing project")
        .children(
          arg[String]("<infile>")
            .required()
            .action((x, c) => c.copy(infile = Some(x)))
            .text("filename should be a .over file to use for the project"),

          opt[String]("instance")
            .action((x, c) => c.copy(instance = Some(x)))
            .text("specify the instance to update")
        ),

      // update catalog subcommand
      cmd("catalog")
        .action((_, c) => c.copy(subCommand = Some("catalog")))
        .text("Update the catalog from the remote repository")
    )

  // Combine all commands and options
  OParser.sequence(
    programName("overlord"),
    head("overlord", "1.0"),
    createCommand,
    generateCommand,
    cleanCommand,
    updateCommand,
    help("help").text("prints this usage text")
  )
}
```

## Command Execution Logic

The main method needs to be updated to handle the new command structure:

```scala
def main(args: Array[String]): Unit = {
  // Parse arguments and set `yes` to true if running in a non-interactive environment
  val initialConfig = Config()
  val isNonInteractive = System.console() == null

  OParser.parse(parser, args, initialConfig.copy(yes = initialConfig.yes || isNonInteractive)) match {
    case Some(config) =>
      // Configure ModuleLogger to exit on error (or not) based on command line option
      ModuleLogger.setExitOnError(!config.noexit)

      // Set log levels
      configureLogging(config)

      // Debug log the parsed configuration
      logConfigDetails(config)

      // Execute the command
      executeCommand(config)

    case None =>
      error(OParser.usage(parser)) // Show dynamically generated usage message
      sys.exit(1)
  }
}

def executeCommand(config: Config): Unit = {
  (config.command, config.subCommand) match {
    // CREATE commands
    case (Some("create"), Some("project")) =>
      executeCreateProject(config)

    case (Some("create"), Some("from-template")) =>
      executeCreateFromTemplate(config)

    // GENERATE commands
    case (Some("generate"), Some("test")) =>
      executeGenerateTest(config)

    case (Some("generate"), Some("report")) =>
      executeGenerateReport(config)

    case (Some("generate"), Some("svd")) =>
      executeGenerateSvd(config)

    // CLEAN commands
    case (Some("clean"), Some("test")) =>
      executeCleanTest(config)

    // UPDATE commands
    case (Some("update"), Some("project")) =>
      executeUpdateProject(config)

    case (Some("update"), Some("catalog")) =>
      executeUpdateCatalog(config)

    // Unknown command combination
    case _ =>
      error("Unknown command or missing subcommand")
      error(OParser.usage(parser))
      sys.exit(1)
  }
}
```

## Command Implementation Methods

Here are the implementation methods for each command:

```scala
def executeCreateProject(config: Config): Unit = {
  // This is similar to the existing "create" command
  val filename = config.infile.getOrElse {
    error("Missing required input file")
    sys.exit(1)
  }

  val board = config.board.getOrElse {
    error("Missing required board option")
    sys.exit(1)
  }

  // Process the file and create the project
  // ... (existing implementation)
}

def executeCreateFromTemplate(config: Config): Unit = {
  val templateName = config.templateName.getOrElse {
    error("Missing required template name")
    sys.exit(1)
  }

  val projectName = config.projectName.getOrElse {
    error("Missing required project name")
    sys.exit(1)
  }

  info(s"Creating project '$projectName' from template '$templateName'")

  // Implementation for creating from template
  // 1. Locate the template
  // 2. Copy template files to the new project directory
  // 3. Customize files with the project name
}

def executeGenerateTest(config: Config): Unit = {
  val projectName = config.projectName.getOrElse {
    error("Missing required project name")
    sys.exit(1)
  }

  info(s"Generating tests for project '$projectName'")

  // Implementation for generating tests
  // 1. Analyze the project structure
  // 2. Create appropriate test files
}

def executeGenerateReport(config: Config): Unit = {
  // This is similar to the existing "report" command
  val filename = config.infile.getOrElse {
    error("Missing required input file")
    sys.exit(1)
  }

  // Process the file and generate the report
  // ... (existing implementation)
}

def executeGenerateSvd(config: Config): Unit = {
  // This is similar to the existing "svd" command
  val filename = config.infile.getOrElse {
    error("Missing required input file")
    sys.exit(1)
  }

  // Process the file and generate the SVD
  // ... (existing implementation)
}

def executeCleanTest(config: Config): Unit = {
  val projectName = config.projectName.getOrElse {
    error("Missing required project name")
    sys.exit(1)
  }

  info(s"Cleaning tests for project '$projectName'")

  // Implementation for cleaning tests
  // 1. Identify test artifacts
  // 2. Remove them
}

def executeUpdateProject(config: Config): Unit = {
  // This is similar to the existing "update" command
  val filename = config.infile.getOrElse {
    error("Missing required input file")
    sys.exit(1)
  }

  // Process the file and update the project
  // ... (existing implementation)
}

def executeUpdateCatalog(config: Config): Unit = {
  info("Updating catalog from remote repository")

  // Implementation for updating the catalog
  // 1. Connect to the remote repository
  // 2. Download the latest definitions
  // 3. Update the local catalog
}
```

## Manager Classes

### TemplateManager

```scala
object TemplateManager {
  private val templateBasePath = Paths.get(System.getProperty("user.home"), ".overlord", "templates")

  def createFromTemplate(templateName: String, projectName: String, outputPath: String): Boolean = {
    info(s"Creating project '$projectName' from template '$templateName'")

    // Ensure the template exists
    val templatePath = templateBasePath.resolve(templateName)
    if (!Files.exists(templatePath)) {
      error(s"Template '$templateName' not found")
      return false
    }

    // Create the output directory
    val projectPath = Paths.get(outputPath, projectName)
    if (Files.exists(projectPath)) {
      error(s"Project directory '$projectPath' already exists")
      return false
    }

    Files.createDirectories(projectPath)

    // Copy template files to the project directory
    copyDirectory(templatePath, projectPath)

    // Customize files with the project name
    customizeFiles(projectPath, projectName)

    info(s"Project '$projectName' created successfully at '$projectPath'")
    true
  }

  def listAvailableTemplates(): List[String] = {
    if (!Files.exists(templateBasePath)) {
      return List.empty
    }

    import scala.jdk.CollectionConverters._
    Files.list(templateBasePath)
      .iterator()
      .asScala
      .filter(Files.isDirectory(_))
      .map(_.getFileName.toString)
      .toList
  }

  private def copyDirectory(source: Path, target: Path): Unit = {
    import scala.jdk.CollectionConverters._

    Files.walk(source).iterator().asScala.foreach { path =>
      val relativePath = source.relativize(path)
      val targetPath = target.resolve(relativePath)

      if (Files.isDirectory(path)) {
        if (!Files.exists(targetPath)) {
          Files.createDirectory(targetPath)
        }
      } else {
        Files.copy(path, targetPath)
      }
    }
  }

  private def customizeFiles(projectPath: Path, projectName: String): Unit = {
    import scala.jdk.CollectionConverters._

    // Find files that need customization
    val filesToCustomize = Files.walk(projectPath)
      .iterator()
      .asScala
      .filter(Files.isRegularFile(_))
      .filter(p => {
        val fileName = p.getFileName.toString
        fileName.endsWith(".over") ||
        fileName.endsWith(".scala") ||
        fileName.endsWith(".md") ||
        fileName == "build.sbt"
      })
      .toList

    // Replace template placeholders with the project name
    filesToCustomize.foreach { file =>
      val content = new String(Files.readAllBytes(file), "UTF-8")
      val customized = content
        .replace("{{project_name}}", projectName)
        .replace("{{PROJECT_NAME}}", projectName.toUpperCase)
        .replace("{{project-name}}", projectName.toLowerCase.replace(" ", "-"))

      Files.write(file, customized.getBytes("UTF-8"))
    }
  }
}
```

### TestManager

```scala
object TestManager {
  def generateTests(projectName: String): Boolean = {
    info(s"Generating tests for project '$projectName'")

    // Find the project directory
    val projectPath = findProjectPath(projectName)
    if (projectPath.isEmpty) {
      error(s"Project '$projectName' not found")
      return false
    }

    // Create test directory if it doesn't exist
    val testPath = projectPath.get.resolve("src").resolve("test")
    if (!Files.exists(testPath)) {
      Files.createDirectories(testPath)
    }

    // Find source files to create tests for
    val sourceFiles = findSourceFiles(projectPath.get)

    // Generate test files
    sourceFiles.foreach { sourceFile =>
      generateTestFile(sourceFile, testPath)
    }

    info(s"Generated ${sourceFiles.size} test files for project '$projectName'")
    true
  }

  def cleanTests(projectName: String): Boolean = {
    info(s"Cleaning tests for project '$projectName'")

    // Find the project directory
    val projectPath = findProjectPath(projectName)
    if (projectPath.isEmpty) {
      error(s"Project '$projectName' not found")
      return false
    }

    // Find and delete test files
    val testPath = projectPath.get.resolve("src").resolve("test")
    if (Files.exists(testPath)) {
      deleteDirectory(testPath)
      info(s"Removed test directory for project '$projectName'")
    } else {
      info(s"No test directory found for project '$projectName'")
    }

    true
  }

  private def findProjectPath(projectName: String): Option[Path] = {
    // Look for the project in the current directory and common locations
    val possibleLocations = List(
      Paths.get(projectName),
      Paths.get(".", projectName),
      Paths.get("..", projectName)
    )

    possibleLocations.find(Files.exists(_))
  }

  private def findSourceFiles(projectPath: Path): List[Path] = {
    import scala.jdk.CollectionConverters._

    val sourcePath = projectPath.resolve("src").resolve("main")
    if (!Files.exists(sourcePath)) {
      return List.empty
    }

    Files.walk(sourcePath)
      .iterator()
      .asScala
      .filter(Files.isRegularFile(_))
      .filter(_.toString.endsWith(".scala"))
      .toList
  }

  private def generateTestFile(sourceFile: Path, testPath: Path): Unit = {
    // Extract package and class name from source file
    val content = new String(Files.readAllBytes(sourceFile), "UTF-8")
    val packageName = extractPackageName(content)
    val className = sourceFile.getFileName.toString.replace(".scala", "")

    // Create test file path
    val testFilePath = testPath.resolve(sourceFile.getFileName.toString.replace(".scala", "Spec.scala"))

    // Generate test file content
    val testContent = s"""package $packageName
                        |
                        |import org.scalatest.flatspec.AnyFlatSpec
                        |import org.scalatest.matchers.should.Matchers
                        |
                        |class ${className}Spec extends AnyFlatSpec with Matchers {
                        |  "$className" should "be tested" in {
                        |    // TODO: Implement test
                        |    true should be(true)
                        |  }
                        |}
                        |""".stripMargin

    // Write test file
    Files.write(testFilePath, testContent.getBytes("UTF-8"))
  }

  private def extractPackageName(content: String): String = {
    val packageRegex = """package\s+([^\s]+)""".r
    packageRegex.findFirstMatchIn(content) match {
      case Some(m) => m.group(1)
      case None => "com.example"
    }
  }

  private def deleteDirectory(path: Path): Unit = {
    import scala.jdk.CollectionConverters._

    if (Files.exists(path)) {
      Files.walk(path)
        .sorted(java.util.Comparator.reverseOrder())
        .iterator()
        .asScala
        .foreach(Files.delete(_))
    }
  }
}
```

### CatalogManager

```scala
object CatalogManager {
  def updateCatalog(): Boolean = {
    info("Updating catalog from remote repository")

    val stdResourcePath = Resources.stdResourcePath()

    // Check if the catalog directory exists
    if (!Files.exists(stdResourcePath)) {
      info(s"Standard resource folder '$stdResourcePath' does not exist. Creating it...")
      Files.createDirectories(stdResourcePath.getParent)
    }

    // Check if it's a git repository
    val isGitRepo = Files.exists(stdResourcePath.resolve(".git"))

    if (isGitRepo) {
      // Update existing repository
      info("Updating existing catalog repository...")

      val pullCommand = Process(
        Seq("git", "pull"),
        new java.io.File(stdResourcePath.toString)
      )

      val pullResult = pullCommand.!
      if (pullResult != 0) {
        error("Error: Failed to update the catalog repository.")
        return false
      }

      // Update submodules
      info("Updating git submodules...")
      val updateCommand = Process(
        Seq("git", "submodule", "update", "--recursive", "--remote"),
        new java.io.File(stdResourcePath.toString)
      )

      val updateResult = updateCommand.!
      if (updateResult != 0) {
        warn("Warning: Failed to update git submodules.")
      } else {
        info("Git submodules successfully updated.")
      }
    } else {
      // Clone the repository
      info("Cloning catalog repository...")

      // Remove the directory if it exists but is not a git repository
      if (Files.exists(stdResourcePath)) {
        info("Removing existing non-git directory...")
        deleteDirectory(stdResourcePath)
      }

      val cloneCommand = s"git clone https://github.com/DeanoC/gagameos_stdcatalog.git $stdResourcePath"
      val cloneResult = cloneCommand.!

      if (cloneResult != 0) {
        error("Error: Failed to clone the catalog repository.")
        return false
      }

      // Initialize and update submodules
      info("Initializing git submodules...")
      val initCommand = Process(
        Seq("git", "submodule", "init"),
        new java.io.File(stdResourcePath.toString)
      )

      val initResult = initCommand.!
      if (initResult != 0) {
        warn("Warning: Failed to initialize git submodules.")
      } else {
        info("Updating git submodules...")
        val updateCommand = Process(
          Seq("git", "submodule", "update", "--recursive"),
          new java.io.File(stdResourcePath.toString)
        )

        val updateResult = updateCommand.!
        if (updateResult != 0) {
          warn("Warning: Failed to update git submodules.")
        } else {
          info("Git submodules successfully initialized and updated.")
        }
      }
    }

    info("Catalog successfully updated.")
    true
  }

  private def deleteDirectory(path: Path): Unit = {
    import scala.jdk.CollectionConverters._

    if (Files.exists(path)) {
      Files.walk(path)
        .sorted(java.util.Comparator.reverseOrder())
        .iterator()
        .asScala
        .foreach(Files.delete(_))
    }
  }
}
```

## Migration Strategy

To implement these changes without disrupting the existing codebase:

1. Create a new branch for the CLI modernization
2. Implement the new command structure in Main.scala
3. Create the manager classes (TemplateManager, TestManager, CatalogManager)
4. Map existing functionality to the new command structure
5. Test thoroughly with various command combinations
6. Update documentation to reflect the new command structure
7. Merge the changes into the main branch

This approach allows for a clean implementation of the new CLI structure while ensuring that existing functionality continues to work.