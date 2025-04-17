# Code Examples for Scala 3 Improvements

This document provides concrete code examples showing how to implement the recommendations from the main review document. These examples are tailored specifically to your Overlord project.

## 1. Functional Error Handling

### Example: Improving ReadYamlRegistersAction

Current implementation:
```scala
// From ReadYamlRegistersAction.scala
def apply(
    name: String,
    process: Map[String, Variant]
): Option[ReadYamlRegistersAction] = {
  // Ensure the process contains a "source" field; otherwise, log a warning and return None.
  if (!process.contains("source")) {
    println(s"Read Yaml Registers process $name doesn't have a source field")
    return None
  }

  // Extract the filename from the "source" field, handling different data types.
  val filename = process("source") match {
    case s: StringV => s.value
    case t: TableV  => Utils.toString(t.value("file"))
    case _          =>
      // Log an error if the "source" field is malformed and return None.
      println("Read Yaml Register source field is malformed")
      return None
  }

  // Return a new instance of ReadYamlRegistersAction.
  Some(ReadYamlRegistersAction(name, filename))
}
```

Improved implementation using Either for error handling:
```scala
// Improved version with Either for better error handling
def apply(
    name: String,
    process: Map[String, Variant]
): Either[String, ReadYamlRegistersAction] = {
  if (!process.contains("source")) {
    Left(s"Read Yaml Registers process $name doesn't have a source field")
  } else {
    // Extract the filename from the "source" field, handling different data types
    process("source") match {
      case s: StringV =>
        Right(ReadYamlRegistersAction(name, s.value))
      case t: TableV if t.value.contains("file") =>
        Right(ReadYamlRegistersAction(name, Utils.toString(t.value("file"))))
      case _ =>
        Left("Read Yaml Register source field is malformed")
    }
  }
}

// Usage example
def createAction(name: String, process: Map[String, Variant]): Unit = {
  ReadYamlRegistersAction(name, process) match {
    case Right(action) =>
      // Use the action
      println(s"Created action: ${action.name}")
    case Left(error) =>
      // Handle the error
      println(s"Error creating action: $error")
  }
}
```

## 2. Immutability and Side Effects

### Example: Refactoring Instance Trait

Current implementation:
```scala
// From Instance.scala
trait InstanceTrait extends QueryInterface {
  lazy val attributes: mutable.HashMap[String, Variant] =
    mutable.HashMap[String, Variant](definition.attributes.toSeq: _*)
  val finalParameterTable: mutable.HashMap[String, Variant] = mutable.HashMap()

  val name: String
  val sourcePath: Path = Project.instancePath.toAbsolutePath

  def definition: DefinitionTrait

  def mergeAllAttributes(attribs: Map[String, Variant]): Unit =
    attribs.foreach(a => mergeAttribute(attribs, a._1))

  def mergeAttribute(attribs: Map[String, Variant], key: String): Unit = {
    if (attribs.contains(key)) {
      // overwrite existing or add it
      attributes.updateWith(key) {
        case Some(_) => Some(attribs(key))
        case None    => Some(attribs(key))
      }
    }
  }
  // ...
}
```

Improved implementation with immutability:
```scala
// Improved version with immutability
trait InstanceTrait extends QueryInterface {
  // Use immutable Map instead of mutable HashMap
  def attributes: Map[String, Variant]
  def finalParameters: Map[String, Variant]

  val name: String
  val sourcePath: Path = Project.instancePath.toAbsolutePath

  def definition: DefinitionTrait

  // Return a new instance with merged attributes instead of mutating
  def withMergedAttributes(attribs: Map[String, Variant]): InstanceTrait

  // Helper method for merging a single attribute
  def withMergedAttribute(attribs: Map[String, Variant], key: String): InstanceTrait = {
    if (attribs.contains(key)) {
      withMergedAttributes(Map(key -> attribs(key)))
    } else {
      this
    }
  }
  // ...
}

// Example implementation for a concrete class
case class ChipInstance(
  name: String,
  definition: DefinitionTrait,
  attributes: Map[String, Variant],
  finalParameters: Map[String, Variant] = Map.empty
) extends InstanceTrait {

  // Implementation that returns a new instance with merged attributes
  def withMergedAttributes(attribs: Map[String, Variant]): ChipInstance = {
    copy(attributes = attributes ++ attribs)
  }

  // Other methods...
}
```

## 3. Pattern Matching and Type Handling

### Example: Improving Type Handling in ReadYamlRegistersAction

Current implementation:
```scala
// From ReadYamlRegistersAction.scala
def execute(
    instance: InstanceTrait,
    parameters: Map[String, Variant]
): Unit = {
  // Check if the instance is a ChipInstance; otherwise, log a warning.
  if (!instance.isInstanceOf[ChipInstance]) {
    println(
      s"${instance.name} is not a chip but is being processed by a gateware action"
    )
  } else {
    // Cast the instance to ChipInstance and execute the action.
    execute(instance.asInstanceOf[ChipInstance], parameters)
  }
}
```

Improved implementation with pattern matching:
```scala
// Improved version with pattern matching
def execute(
    instance: InstanceTrait,
    parameters: Map[String, Variant]
): Unit = {
  instance match {
    case chipInstance: ChipInstance =>
      execute(chipInstance, parameters)
    case _ =>
      println(s"${instance.name} is not a chip but is being processed by a gateware action")
  }
}
```

## 4. Command-Line Argument Parsing

### Example: Refactoring Main.scala with scopt

Current implementation:
```scala
// From Main.scala
def main(args: Array[String]): Unit = {
  if (args.isEmpty) {
    println(usage)
    println("No arguments provided.")
    sys.exit(1)
  }
  type OptionMap = Map[Symbol, Any]

  val initialOptions: Map[Symbol, Any] =
    MainUtils.nextOption(Map(), args.toList)
  val options = initialOptions.updated(
    Symbol("yes"),
    initialOptions.contains(Symbol("yes")) || System.console() == null
  )
  if (!options.contains(Symbol("infile"))) {
    println(usage)
    println(s"Arguments passed: ${args.mkString(" ")}")
    println("Error: filename is required")
    sys.exit(1)
  }
  // ...
}

// Complex recursive argument parsing
@tailrec
def nextOption(map: OptionMap, list: List[String]): OptionMap = {
  def isSwitch(s: String) = s.startsWith("-")

  list match {
    case Nil =>
      if (!map.contains(Symbol("infile")))
        throw new IllegalArgumentException("Missing required option: infile")
      if (!map.contains(Symbol("board")))
        throw new IllegalArgumentException("Missing required option: board")
      map
    case "create" :: tail =>
      nextOption(map + (Symbol("create") -> true), tail)
    // Many more cases...
  }
}
```

Improved implementation with scopt:
```scala
// Add to build.sbt:
// libraryDependencies += "com.github.scopt" %% "scopt" % "4.1.0"

import scopt.OParser

// Configuration case class
case class OverlordConfig(
  mode: String = "",
  outPath: String = ".",
  board: String = "",
  useStdResources: Boolean = true,
  useStdPrefabs: Boolean = true,
  resourcesPath: Option[String] = None,
  instanceName: Option[String] = None,
  autoAgree: Boolean = false,
  infile: String = ""
)

object Main {
  def main(args: Array[String]): Unit = {
    val builder = OParser.builder[OverlordConfig]
    val parser = {
      import builder._
      OParser.sequence(
        programName("overlord"),
        head("overlord", "1.0"),

        // Commands
        cmd("create")
          .action((_, c) => c.copy(mode = "create"))
          .text("generate a compile project with all sub parts at 'out'"),
        cmd("update")
          .action((_, c) => c.copy(mode = "update"))
          .text("update an existing project instance"),
        cmd("report")
          .action((_, c) => c.copy(mode = "report"))
          .text("prints some info about the overall structure"),
        cmd("svd")
          .action((_, c) => c.copy(mode = "svd"))
          .text("produce a CMSIS-SVD file on its own"),

        // Options
        opt[String]('o', "out")
          .action((x, c) => c.copy(outPath = x))
          .text("path where generated files should be placed"),
        opt[String]("board")
          .required()
          .action((x, c) => c.copy(board = x))
          .text("board definition to use"),
        opt[Unit]("nostdresources")
          .action((_, c) => c.copy(useStdResources = false))
          .text("don't use the standard catalog"),
        opt[Unit]("nostdprefabs")
          .action((_, c) => c.copy(useStdPrefabs = false))
          .text("don't use the standard prefabs"),
        opt[String]("resources")
          .action((x, c) => c.copy(resourcesPath = Some(x)))
          .text("use the specified path as the root of resources"),
        opt[String]("instance")
          .action((x, c) => c.copy(instanceName = Some(x)))
          .text("specify the instance to update"),
        opt[Unit]('y', "yes")
          .action((_, c) => c.copy(autoAgree = true))
          .text("automatically agree (i.e. automatically download resource files without prompting)"),

        // Arguments
        arg[String]("<filename>")
          .required()
          .action((x, c) => c.copy(infile = x))
          .text("a .yaml file to use for the project")
      )
    }

    // Parse arguments
    OParser.parse(parser, args, OverlordConfig()) match {
      case Some(config) =>
        // Use the parsed configuration
        runWithConfig(config)
      case _ =>
        // Arguments are bad, error message will have been displayed
        sys.exit(1)
    }
  }

  private def runWithConfig(config: OverlordConfig): Unit = {
    // Implementation using the parsed config
    val filename = config.infile
    if (!Files.exists(Paths.get(filename))) {
      println(s"Error: $filename does not exist")
      sys.exit(1)
    }

    // Rest of the implementation...
  }
}
```

## 5. Scala 3 Specific Features

### Example: Using Enums for Action Types

Current approach (implicit string-based types):
```scala
// Implicit type handling through strings
val actionType = "ReadYamlRegisters"
// Later used in conditionals
if (actionType == "ReadYamlRegisters") {
  // Do something
} else if (actionType == "ShellAction") {
  // Do something else
}
```

Improved implementation with Scala 3 enums:
```scala
// Define an enum for action types
enum ActionType:
  case ReadYamlRegisters, ShellAction, GitClone, Template, Copy, Python, SoftSource

  // Add a method to get a string representation
  def asString: String = this match
    case ReadYamlRegisters => "ReadYamlRegisters"
    case ShellAction => "ShellAction"
    case GitClone => "GitClone"
    case Template => "Template"
    case Copy => "Copy"
    case Python => "Python"
    case SoftSource => "SoftSource"

// Parse a string to get an ActionType
object ActionType:
  def fromString(str: String): Option[ActionType] = str match
    case "ReadYamlRegisters" => Some(ReadYamlRegisters)
    case "ShellAction" => Some(ShellAction)
    case "GitClone" => Some(GitClone)
    case "Template" => Some(Template)
    case "Copy" => Some(Copy)
    case "Python" => Some(Python)
    case "SoftSource" => Some(SoftSource)
    case _ => None

// Usage
val actionType = ActionType.ReadYamlRegisters
// Type-safe pattern matching
actionType match
  case ActionType.ReadYamlRegisters =>
    // Do something
  case ActionType.ShellAction =>
    // Do something else
  case _ =>
    // Handle other cases
```

### Example: Using Extension Methods

```scala
// Add extension methods for Path
extension (path: Path)
  def resolveAndNormalize(other: String): Path =
    path.resolve(other).normalize()

  def isValidGitRepo: Boolean =
    Files.exists(path.resolve(".git")) && Files.isDirectory(path.resolve(".git"))

  def ensureDirectories(): Path =
    Files.createDirectories(path)
    path

// Usage
val configPath = projectPath.resolveAndNormalize("config")
if (!configPath.isValidGitRepo) {
  println("Not a valid git repository")
}
val outputPath = outPath.resolveAndNormalize("output").ensureDirectories()
```

### Example: Using Opaque Type Aliases

```scala
// Define opaque types for domain concepts
object DomainTypes:
  opaque type BoardName = String
  object BoardName:
    def apply(name: String): BoardName = name
    extension (b: BoardName) def value: String = b

  opaque type InstanceName = String
  object InstanceName:
    def apply(name: String): InstanceName = name
    extension (i: InstanceName) def value: String = i

// Usage
import DomainTypes.*

def createProject(board: BoardName, instance: InstanceName): Unit = {
  println(s"Creating project for board ${board.value} with instance ${instance.value}")
  // Implementation...
}

// Type safety - can't mix up board and instance names
val board = BoardName("myBoard")
val instance = InstanceName("myInstance")
createProject(board, instance)
```

## 6. Dependency Injection

### Example: Refactoring YamlRegistersParser

Current implementation:
```scala
// From YamlRegistersParser.scala
object YamlRegistersParser {
  def apply(
      instance: InstanceTrait,
      filename: String,
      name: String
  ): Seq[RegisterBank] = {
    println(s"parsing $name yaml for register definitions")
    val regSeq = Seq(
      TableV(
        Map(
          "resource" -> StringV(filename),
          "name" -> StringV(name),
          "base_address" -> BigIntV(0)
        )
      )
    )
    Registers(instance, regSeq)
  }
}

// Usage in ReadYamlRegistersAction
def execute(
    instance: ChipInstance,
    parameters: Map[String, Variant]
): Unit = {
  val expandedName = Project.resolveInstanceMacros(instance, filename)
  val registers = YamlRegistersParser(instance, expandedName, instance.name)
  instance.instanceRegisterBanks ++= registers
}
```

Improved implementation with dependency injection:
```scala
// Define a trait for the parser
trait RegistersParser:
  def parse(instance: InstanceTrait, filename: String, name: String): Seq[RegisterBank]

// Concrete implementation
class YamlRegistersParser extends RegistersParser:
  def parse(instance: InstanceTrait, filename: String, name: String): Seq[RegisterBank] = {
    println(s"parsing $name yaml for register definitions")
    val regSeq = Seq(
      TableV(
        Map(
          "resource" -> StringV(filename),
          "name" -> StringV(name),
          "base_address" -> BigIntV(0)
        )
      )
    )
    Registers(instance, regSeq)
  }

// Default implementation for backward compatibility
object YamlRegistersParser:
  private val defaultParser = new YamlRegistersParser()

  def apply(
      instance: InstanceTrait,
      filename: String,
      name: String
  ): Seq[RegisterBank] = defaultParser.parse(instance, filename, name)

// Modified ReadYamlRegistersAction with dependency injection
class ReadYamlRegistersAction(
    name: String,
    filename: String,
    parser: RegistersParser = new YamlRegistersParser() // Default implementation
) extends GatewareAction {

  override val phase: Int = 2

  def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    instance match {
      case chipInstance: ChipInstance =>
        execute(chipInstance, parameters)
      case _ =>
        println(s"${instance.name} is not a chip but is being processed by a gateware action")
    }
  }

  override def execute(
      instance: ChipInstance,
      parameters: Map[String, Variant]
  ): Unit = {
    val expandedName = Project.resolveInstanceMacros(instance, filename)
    val registers = parser.parse(instance, expandedName, instance.name)
    instance.instanceRegisterBanks ++= registers
  }
}
```

## 7. Logging

### Example: Adding Proper Logging to ShellAction

Current implementation:
```scala
// From ShellAction.scala
override def execute(
    instance: InstanceTrait,
    parameters: Map[String, Variant]
): Unit = {
  import scala.language.postfixOps

  // Resolve macros in the arguments and execute the bash script
  val result = sys.process
    .Process(
      Seq("bash", s"$script", Project.resolvePathMacros(instance, s"$args")),
      Project.catalogPath.toFile
    )
    .!

  // Check if the script execution failed
  if (result != 0) println(s"FAILED bash of $script $args")
}
```

Improved implementation with proper logging:
```scala
// Add to build.sbt:
// libraryDependencies += "com.typesafe.scala-logging" %% "scala-logging" % "3.9.5"
// libraryDependencies += "ch.qos.logback" % "logback-classic" % "1.4.7"

import com.typesafe.scalalogging.Logger

case class ShellAction(script: String, args: String, phase: Int = 1)
    extends Action {

  private val logger = Logger[ShellAction]

  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    import scala.language.postfixOps

    val resolvedArgs = Project.resolvePathMacros(instance, s"$args")
    logger.debug(s"Executing bash script: $script with args: $resolvedArgs")

    // Resolve macros in the arguments and execute the bash script
    val result = sys.process
      .Process(
        Seq("bash", s"$script", resolvedArgs),
        Project.catalogPath.toFile
      )
      .!

    // Check if the script execution failed
    if (result != 0) {
      logger.error(s"FAILED bash of $script $args (exit code: $result)")
    } else {
      logger.debug(s"Successfully executed bash script: $script")
    }
  }
}
```

## 8. Code Organization

### Example: Reorganizing the Actions Package

Current structure:
```
src/main/scala/
  └── actions/
      ├── ActionsFile.scala
      ├── CopyAction.scala
      ├── GitCloneAction.scala
      ├── PythonAction.scala
      ├── ReadVerilogTopAction.scala
      ├── ReadYamlRegistersAction.scala
      ├── SbtAction.scala
      ├── ShellAction.scala
      ├── SoftSourceAction.scala
      ├── SourcesAction.scala
      └── TemplateAction.scala
```

Improved structure:
```
src/main/scala/
  └── actions/
      ├── core/
      │   ├── Action.scala          # Base trait for all actions
      │   ├── GatewareAction.scala  # Base trait for gateware actions
      │   └── SoftwareAction.scala  # Base trait for software actions
      ├── file/
      │   ├── CopyAction.scala
      │   └── TemplateAction.scala
      ├── external/
      │   ├── GitCloneAction.scala
      │   ├── PythonAction.scala
      │   ├── SbtAction.scala
      │   └── ShellAction.scala
      ├── parsing/
      │   ├── ReadVerilogTopAction.scala
      │   ├── ReadYamlRegistersAction.scala
      │   └── YamlAction.scala
      └── ActionsFile.scala         # Top-level actions file parser
```

## 9. Testing

### Example: Adding Tests for ReadYamlRegistersAction

```scala
// Add to build.sbt:
// libraryDependencies += "org.scalatest" %% "scalatest" % "3.2.17" % "test"
// libraryDependencies += "org.mockito" % "mockito-core" % "5.3.1" % "test"

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers
import org.mockito.Mockito._
import org.mockito.ArgumentMatchers._

class ReadYamlRegistersActionSpec extends AnyFlatSpec with Matchers {

  "ReadYamlRegistersAction.apply" should "return None when source field is missing" in {
    // Setup
    val name = "testAction"
    val process = Map[String, Variant]()

    // Execute
    val result = ReadYamlRegistersAction(name, process)

    // Verify
    result shouldBe None
  }

  it should "return Some(ReadYamlRegistersAction) when source is a StringV" in {
    // Setup
    val name = "testAction"
    val filename = "test.yaml"
    val process = Map[String, Variant]("source" -> StringV(filename))

    // Execute
    val result = ReadYamlRegistersAction(name, process)

    // Verify
    result shouldBe defined
    result.get.name shouldBe name
    result.get.filename shouldBe filename
  }

  "ReadYamlRegistersAction.execute" should "add registers to the instance" in {
    // Setup
    val name = "testAction"
    val filename = "test.yaml"
    val action = ReadYamlRegistersAction(name, filename)

    val mockInstance = mock(classOf[ChipInstance])
    val mockRegisterBanks = mock(classOf[mutable.Buffer[RegisterBank]])
    when(mockInstance.name).thenReturn("testInstance")
    when(mockInstance.instanceRegisterBanks).thenReturn(mockRegisterBanks)

    val parameters = Map[String, Variant]()

    // Execute
    action.execute(mockInstance, parameters)

    // Verify
    verify(mockRegisterBanks).++=(any[Seq[RegisterBank]])
  }
}
```

## 10. Documentation

### Example: Adding Comprehensive Scaladoc

```scala
/**
 * Represents an action to read YAML-defined registers and associate them with a chip instance.
 *
 * This action parses a YAML file containing register definitions and adds them to the
 * register banks of a chip instance. It is typically used during the hardware definition
 * phase of a project.
 *
 * @param name The name of the action, used for identification and logging
 * @param filename The path to the YAML file containing register definitions
 */
case class ReadYamlRegistersAction(name: String, filename: String)
    extends GatewareAction {

  /**
   * The execution phase for this action.
   * Phase 2 indicates that this action should be executed after basic initialization
   * but before final output generation.
   */
  override val phase: Int = 2

  /**
   * Executes the action for a given instance and parameters.
   *
   * This method checks if the provided instance is a ChipInstance and delegates
   * to the appropriate execute method. If the instance is not a ChipInstance,
   * a warning is logged.
   *
   * @param instance The instance to execute the action on
   * @param parameters Additional parameters for the execution
   */
  def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    instance match {
      case chipInstance: ChipInstance =>
        execute(chipInstance, parameters)
      case _ =>
        println(s"${instance.name} is not a chip but is being processed by a gateware action")
    }
  }

  /**
   * Executes the action specifically for a ChipInstance.
   *
   * This method resolves macros in the filename, parses the YAML file to retrieve
   * register definitions, and appends the parsed registers to the instance's register banks.
   *
   * @param instance The chip instance to execute the action on
   * @param parameters Additional parameters for the execution
   */
  override def execute(
      instance: ChipInstance,
      parameters: Map[String, Variant]
  ): Unit = {
    // Resolve macros in the filename based on the instance context
    val expandedName = Project.resolveInstanceMacros(instance, filename)
    // Parse the YAML file to retrieve register definitions
    val registers = YamlRegistersParser(instance, expandedName, instance.name)
    // Append the parsed registers to the instance's register banks
    instance.instanceRegisterBanks ++= registers
  }
}

/**
 * Companion object for ReadYamlRegistersAction.
 *
 * Provides factory methods to create ReadYamlRegistersAction instances from
 * process definitions.
 */
object ReadYamlRegistersAction {
  /**
   * Creates a ReadYamlRegistersAction from a process definition.
   *
   * @param name The name of the action
   * @param process A map containing the process definition
   * @return Some(ReadYamlRegistersAction) if the process definition is valid, None otherwise
   */
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Option[ReadYamlRegistersAction] = {
    // Implementation...
  }
}
```
