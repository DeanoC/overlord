package com.deanoc.overlord.actions

import com.deanoc.overlord.utils._
import com.deanoc.overlord.Project
import com.deanoc.overlord.Instances.InstanceTrait

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
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Either[String, Seq[ShellAction]] = {
    if (!process.contains("script"))
      Left(s"Shell process $name doesn't have a script field")
    else if (!process("script").isInstanceOf[StringV])
      Left(s"Shell process $name script isn't a string")
    else if (!process.contains("args"))
      Left(s"Shell process $name doesn't have an args field")
    else if (!process("args").isInstanceOf[StringV])
      Left(s"Shell process $name args isn't a string")
    else {
      val script = Utils.toString(process("script"))
      val args = Utils.toString(process("args"))
      Right(Seq(ShellAction(script, args)))
    }
  }

  // Legacy method for backward compatibility
  def fromProcess(
      name: String,
      process: Map[String, Variant]
  ): Seq[ShellAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) =>
        println(errorMsg)
        Seq.empty
    }
  }
}
