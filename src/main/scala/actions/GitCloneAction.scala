package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

// Represents an action to clone a Git repository
case class GitCloneAction(url: String) extends Action {

  // Defines the execution phase for this action
  override val phase: Int = 1

  // Executes the Git clone operation for the given instance and parameters
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    import scala.language.postfixOps
    import scala.sys.process._

    // Determine the output path for the cloned repository
    val path = Project.outPath.resolve(url.split('/').last)
    if (!path.toFile.exists()) {
      // Clone the repository recursively
      val result = s"git clone --recursive $url $path".!

      // Check if the clone operation failed
      if (result != 0) {
        println(s"FAILED git clone of $url")
        return
      }

      // Update submodules recursively
      val submoduleResult =
        s"git -C $path submodule update --init --recursive".!
      if (submoduleResult != 0) {
        println(s"FAILED to update submodules for $url")
      }
    }
  }
}

object GitCloneAction {
  // Factory method to create GitCloneAction instances from a process map
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Seq[GitCloneAction] = {
    // Ensure the process map contains a "url" field
    if (!process.contains("url")) {
      println(s"Git Clone process $name doesn't have a url field")
      return Seq()
    }

    // Placeholder for additional logic (if needed)
    ???

    // Create a GitCloneAction instance using the URL from the process map
    Seq(GitCloneAction(Utils.toString(process("url"))))
  }
}
