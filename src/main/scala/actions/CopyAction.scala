package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

import java.nio.file.{Path, Paths}

// Represents an action to copy a file from a source path to a destination path
case class CopyAction(filename: String, language: String, srcPath: String)
    extends Action {

  // Defines the execution phase of this action
  override val phase: Int = 1

  // Stores the absolute path of the destination file
  private var dstAbsPath: Path = Paths.get("")

  // Executes the copy action for a given instance and parameters
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    // Extracts the filename from the full path
    val fn = filename.split('/').last

    // Resolves the absolute source path using project macros
    val srcAbsPath = Project.projectPath
      .resolve(Project.resolvePathMacros(instance, srcPath))
      .toAbsolutePath

    // Constructs the absolute destination path
    dstAbsPath = Project.outPath.resolve(s"${instance.name}/$fn").toAbsolutePath

    // Ensures that the destination directory exists
    Utils.ensureDirectories(dstAbsPath.getParent)

    // Reads the source file and writes its content to the destination
    val source = Utils.readFile(srcAbsPath)
    Utils.writeFile(dstAbsPath, source.toString)
  }

  // Returns the destination path as a string with forward slashes
  def getDestPath: String = dstAbsPath.toString.replace('\\', '/')
}

object CopyAction {
  // Factory method to create a sequence of CopyAction instances from a process map
  def apply(name: String, process: Map[String, Variant]): Seq[CopyAction] = {
    // Checks if the "sources" field exists in the process map
    if (!process.contains("sources")) {
      println(s"Sources process $name doesn't have a sources field")
      return Seq()
    }

    // Converts the "sources" field into an array of entries
    val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

    // Creates a CopyAction for each entry in the sources array
    for (entry <- srcs.toIndexedSeq) yield {
      val filename = Utils.toString(entry("file"))
      CopyAction(filename, Utils.toString(entry("language")), filename)
    }
  }
}
