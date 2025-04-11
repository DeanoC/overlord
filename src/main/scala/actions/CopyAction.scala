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
  def apply(name: String, process: Map[String, Variant]): Either[String, Seq[CopyAction]] = {
    // Checks if the "sources" field exists in the process map
    if (!process.contains("sources")) {
      Left(s"Sources process $name doesn't have a sources field")
    } else {
      try {
        // Converts the "sources" field into an array of entries
        val srcs = Utils.toArray(process("sources")).map(Utils.toTable)
        
        // Check for required fields in all entries first
        val missingFields = srcs.zipWithIndex.collectFirst {
          case (entry, idx) if !entry.contains("file") => 
            Left(s"Source entry #${idx+1} in $name is missing the 'file' field")
          case (entry, idx) if !entry.contains("language") => 
            Left(s"Source entry for ${Utils.toString(entry("file"))} in $name is missing the 'language' field")
        }
        
        missingFields match {
          case Some(error) => error
          case None =>
            // Creates a CopyAction for each entry in the sources array
            val actions = for (entry <- srcs.toIndexedSeq) yield {
              val filename = Utils.toString(entry("file"))
              CopyAction(filename, Utils.toString(entry("language")), filename)
            }
            Right(actions)
        }
      // Handle any exceptions that occur during processing
      } catch {
        case e: Exception => Left(s"Error processing sources in $name: ${e.getMessage}")
      }
    }
  }
  
  // Legacy method for backward compatibility
  def fromProcess(name: String, process: Map[String, Variant]): Seq[CopyAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) => 
        println(errorMsg)
        Seq.empty
    }
  }
}
