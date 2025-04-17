package com.deanoc.overlord.actions

import com.deanoc.overlord.utils._
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.instances.InstanceTrait

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
    val srcNamePath = Overlord.tryPaths(instance, filename)
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
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Either[String, Seq[SourcesAction]] = {
    // Check if the process map contains a "sources" field.
    if (!process.contains("sources")) {
      Left(s"Sources process $name doesn't have a sources field")
    } else {
      try {
        // Convert the "sources" field into an array of source entries.
        val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

        // Check for required fields in all entries first
        val missingFields = srcs.zipWithIndex.collectFirst {
          case (entry, idx) if !entry.contains("file") =>
            Left(
              s"Source entry #${idx + 1} in $name is missing the 'file' field"
            )
          case (entry, idx) if !entry.contains("language") =>
            Left(
              s"Source entry for ${Utils.toString(entry("file"))} in $name is missing the 'language' field"
            )
        }

        missingFields match {
          case Some(error) => error
          case None        =>
            // Create a SourcesAction instance for each source entry.
            val actions = for (entry <- srcs.toIndexedSeq) yield {
              val filename = Utils.toString(entry("file"))
              SourcesAction(filename, Utils.toString(entry("language")))
            }

            Right(actions)
        }
      } catch {
        case e: Exception =>
          Left(s"Error processing sources in $name: ${e.getMessage}")
      }
    }
  }

  // Legacy method for backward compatibility
  def fromProcess(
      name: String,
      process: Map[String, Variant]
  ): Seq[SourcesAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) =>
        println(errorMsg)
        Seq.empty
    }
  }
}
