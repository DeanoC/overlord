package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

// Represents an action to execute a Python script with arguments
case class PythonAction(script: String, args: String) extends Action {

  // Defines the execution phase for this action
  override val phase: Int = 1

  // Executes the Python script with the provided arguments
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    import scala.language.postfixOps

    // Resolve macros in the arguments and execute the Python script
    val result = sys.process
      .Process(
        Seq(
          "python3",
          s"$script",
          Project.resolvePathMacros(instance, s"$args")
        ),
        Project.catalogPath.toFile
      )
      .!

    // Check if the script execution failed
    if (result != 0) println(s"FAILED python3 of $script $args")
  }
}

object PythonAction {
  // Factory method to create PythonAction instances from a process map
  def apply(name: String, process: Map[String, Variant]): Seq[PythonAction] = {
    // Ensure the process map contains a "script" field
    if (!process.contains("script")) {
      println(s"Python process $name doesn't have a script field")
      None
    }
    // Ensure the "script" field is a string
    if (!process("script").isInstanceOf[StringV]) {
      println(s"Python process $name script isn't a string")
      None
    }
    // Ensure the process map contains an "args" field
    if (!process.contains("args")) {
      println(s"Python process $name doesn't have a args field")
      None
    }
    // Ensure the "args" field is a string
    if (!process("args").isInstanceOf[StringV]) {
      println(s"Python process $name args isn't a string")
      None
    }
    // Extract the script and arguments from the process map
    val script = Utils.toString(process("script"))
    val args = Utils.toString(process("args"))
    // Create a PythonAction instance
    Seq(PythonAction(script, args))
  }
}
