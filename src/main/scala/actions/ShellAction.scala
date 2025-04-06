package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

case class ShellAction(script: String, args: String, phase: Int = 1)
    extends Action {
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
}

object ShellAction {
  // Factory method to create ShellAction instances from a process map
  def apply(name: String, process: Map[String, Variant]): Seq[ShellAction] = {
    // Ensure the process map contains a "script" field
    if (!process.contains("script")) {
      println(s"Shell process $name doesn't have a script field")
      return Seq.empty
    }
    // Ensure the "script" field is a string
    if (!process("script").isInstanceOf[StringV]) {
      println(s"Shell process $name script isn't a string")
      return Seq.empty
    }
    // Ensure the process map contains an "args" field
    if (!process.contains("args")) {
      println(s"Shell process $name doesn't have an args field")
      return Seq.empty
    }
    // Ensure the "args" field is a string
    if (!process("args").isInstanceOf[StringV]) {
      println(s"Shell process $name args isn't a string")
      return Seq.empty
    }
    // Extract the script and arguments from the process map
    val script = Utils.toString(process("script"))
    val args = Utils.toString(process("args"))
    // Create a ShellAction instance
    Seq(ShellAction(script, args))
  }
}
