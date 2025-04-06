package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

// Represents an SBT action with parameters for the main Scala file, arguments, and source path.
case class SbtAction(mainScala: String, args: String, srcPath: String)
    extends Action {

  // Defines the execution phase for this action.
  override val phase: Int = 1

  // Executes the SBT action for a given instance with provided parameters.
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    import scala.language.postfixOps

    // Resolves the absolute path of the source directory.
    val srcAbsPath = Project.tryPaths(instance, srcPath)
    // Resolves the output path for the modified files.
    val moddedOutPath = Project.outPath.resolve(instance.name)

    // Ensures the output directory exists.
    Utils.ensureDirectories(moddedOutPath)

    // Copies all files from the source directory to the output directory.
    srcAbsPath.toFile.listFiles.foreach(f =>
      if (f.isFile)
        Utils.copy(f.toPath, moddedOutPath.resolve(f.toPath.getFileName))
    )

    // Resolves macros in the arguments string.
    val proargs = Project.resolvePathMacros(instance, args)

    // Executes the SBT process with the resolved arguments in the output directory.
    val result =
      sys.process.Process(Seq("sbt", proargs), moddedOutPath.toFile).!

    // Logs a failure message if the SBT process fails.
    if (result != 0) println(s"FAILED sbt of ${instance.name} with $args")
  }
}

object SbtAction {
  // Factory method to create a sequence of SbtAction instances from a process definition.
  def apply(name: String, process: Map[String, Variant]): Seq[SbtAction] = {
    // Validates the presence of required fields in the process definition.
    if (!process.contains("args")) {
      println(s"SBT process $name doesn't have a args field")
      None
    }
    if (!process.contains("main_scala")) {
      println(s"SBT process $name doesn't have a main_scala field")
      None
    }

    // Validates that the "args" field is a string.
    if (!process("args").isInstanceOf[StringV]) {
      println(s"SBT process $name args isn't a string")
      None
    }

    // Checks if the process includes a "with_build_sbt" flag and converts it to a boolean.
    val withBuildSbt =
      if (process.contains("with_build_sbt"))
        Utils.toBoolean(process("with_build_sbt"))
      else false

    // Extracts the main Scala file and arguments from the process definition.
    val mainScala = Utils.toString(process("main_scala"))
    val args = Utils.toString(process("args"))

    // Creates a sequence of SbtAction instances with the extracted parameters.
    Seq(SbtAction(mainScala, args, Project.catalogPath.toString))
  }
}
