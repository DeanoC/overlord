package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

// Represents an action to handle source files with a specific filename and language.
case class SourcesAction(filename: String, language: String) extends Action {

  // Defines the execution phase of this action.
  override val phase: Int = 1

  // Stores the resolved absolute path of the source file.
  var actualSrcPath: String = ""

  // Executes the action by resolving the source file path and checking its existence.
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    // Attempt to resolve the source file path using the project paths.
    val srcNamePath = Project.tryPaths(instance, filename)
    val srcAbsPath = srcNamePath.toAbsolutePath

    // Check if the resolved file exists, and log a message if it doesn't.
    if (!srcAbsPath.toFile.exists()) {
      println(f"SourceAction: $srcAbsPath not found%n")
    }

    // Store the resolved absolute path.
    actualSrcPath = srcAbsPath.toString // TODO clean this
  }

  // Returns the resolved absolute path of the source file.
  def getSrcPath: String = actualSrcPath

}

object SourcesAction {
  // Factory method to create a sequence of SourcesAction instances from a process map.
  def apply(name: String, process: Map[String, Variant]): Seq[SourcesAction] = {
    // Check if the process map contains a "sources" field.
    if (!process.contains("sources")) {
      println(s"Sources process $name doesn't have a sources field")
      return Seq()
    }

    // Convert the "sources" field into an array of source entries.
    val srcs = Utils.toArray(process("sources")).map(Utils.toTable)
    // Create a SourcesAction instance for each source entry.
    for (entry <- srcs.toIndexedSeq) yield {
      val filename = Utils.toString(entry("file"))
      SourcesAction(filename, Utils.toString(entry("language")))
    }
  }
}
