package com.deanoc.overlord.actions

import com.deanoc.overlord.utils.{Variant, ArrayV, Utils}
import com.deanoc.overlord.Instances.{ChipInstance, InstanceTrait}
import scala.util.boundary, boundary.break

// Represents a generic action with a phase and an execution method.
trait Action {
  val phase: Int

  // Executes the action with the given instance and parameters.
  def execute(instance: InstanceTrait, parameters: Map[String, Variant]): Unit
}

// Represents an action specific to gateware with a specialized execution method.
trait GatewareAction extends Action {
  def execute(instance: ChipInstance, parameters: Map[String, Variant]): Unit
}

// Represents an action specific to software.
trait SoftwareAction extends Action

// Encapsulates a collection of actions.
case class ActionsFile(actions: Seq[Action])

object ActionsFile {
  // Factory method to create an ActionsFile from a name and parsed data.
  def apply(
      name: String,
      parsed: Map[String, Variant]
  ): Either[String, ActionsFile] = {
    // Check if the parsed data contains required fields
    if (
      !parsed.contains("actions") || !parsed("actions").isInstanceOf[ArrayV]
    ) {
      Left(s"$name doesn't have an actions field")
    } else if (
      !parsed.contains("process") || !parsed("process").isInstanceOf[ArrayV]
    ) {
      Left(s"$name doesn't have process fields")
    } else {
      try {
        // Extract processes from the parsed data
        val processEntries = Utils
          .toArray(parsed("process"))
          .map(Utils.toTable)
          .filter(_.contains("name"))
          .map(tbl => Utils.toString(tbl("name")) -> tbl)
          .toMap

        // Process each action and collect results
        val actionResults = Utils.toArray(parsed("actions")).map { taction =>
          val actionName = Utils.toString(taction)

          // Check if process exists
          if (!processEntries.contains(actionName)) {
            Left(s"Action process $actionName not found in $name")
          } else {
            val process = processEntries(actionName)

            // Check if processor type exists
            if (!process.contains("processor")) {
              Left(
                s"Process $actionName in $name doesn't have a processor field"
              )
            } else {
              // Create appropriate action based on processor type
              Utils.toString(process("processor")) match {
                case "copy"             => CopyAction(name, process)
                case "sources"          => SourcesAction(name, process)
                case "git"              => GitCloneAction(name, process)
                case "shell"            => ShellAction(name, process)
                case "python"           => PythonAction(name, process)
                case "yaml"             => YamlAction(name, process)
                case "sbt"              => SbtAction(name, process)
                case "read_verilog_top" => ReadVerilogTopAction(name, process)
                case "read_yaml_registers" =>
                  ReadYamlRegistersAction(name, process)
                case "templates"        => TemplateAction(name, process)
                case "software_sources" => SoftSourceAction(name, process)
                case processorType =>
                  Left(s"Unknown action processor ($processorType) in $name")
              }
            }
          }
        }

        // Check for any errors in processing actions
        val firstError = actionResults.collectFirst { case Left(error) =>
          error
        }

        firstError match {
          case Some(error) => Left(error)
          case None        =>
            // Combine all action sequences into one
            val combinedActions = actionResults.collect {
              case Right(actionSeq) => actionSeq
            }.flatten

            Right(ActionsFile(combinedActions.toIndexedSeq))
        }
      } catch {
        case e: Exception =>
          Left(s"Error processing actions file $name: ${e.getMessage}")
      }
    }
  }

  // Legacy method for backward compatibility
  def createActionsFile(
      name: String,
      parsed: Map[String, Variant]
  ): Option[ActionsFile] = {
    apply(name, parsed) match {
      case Right(actionsFile) => Some(actionsFile)
      case Left(errorMsg) =>
        println(errorMsg)
        None
    }
  }
}
