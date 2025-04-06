package actions

import gagameos._
import overlord.Instances.{ChipInstance, InstanceTrait}
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
  def apply(name: String, parsed: Map[String, Variant]): Option[ActionsFile] = {
    // Check if the parsed data contains an "actions" field of type ArrayV.
    if (
      !parsed.contains("actions") ||
      !parsed("actions").isInstanceOf[ArrayV]
    ) {
      println(s"$name doesn't have an actions field")
      return None
    }

    // Check if the parsed data contains a "process" field of type ArrayV.
    if (
      !parsed.contains("process") ||
      !parsed("process").isInstanceOf[ArrayV]
    ) {
      println(s"$name doesn't have process fields")
      return None
    }

    // Extract processes from the parsed data.
    val processes =
      (for {
        tprocess <- Utils.toArray(parsed("process"))
        tbl = Utils.toTable(tprocess)
      } yield {
        // Ensure each process has a "name" field.
        if (tbl.contains("name"))
          Some(Utils.toString(tbl("name")) -> tbl)
        else None
      }).flatten.toMap

    // Extract actions and map them to their corresponding processes.
    val actions =
      (for {
        taction <- Utils.toArray(parsed("actions"))
        action = Utils.toString(taction)
      } yield {

        boundary {
          // Ensure the action references a valid process.
          if (!processes.contains(action)) {
            println(s"$action process not found in $name")
            break(None)
          }

          val process = processes(action)
          // Match the processor type to create the appropriate action.
          Utils.toString(process("processor")) match {
            case "copy"                => CopyAction(name, process)
            case "sources"             => SourcesAction(name, process)
            case "git"                 => GitCloneAction(name, process)
            case "python"              => PythonAction(name, process)
            case "yaml"                => YamlAction(name, process)
            case "sbt"                 => SbtAction(name, process)
            case "read_verilog_top"    => ReadVerilogTopAction(name, process)
            case "read_yaml_registers" => ReadYamlRegistersAction(name, process)
            case "templates"           => TemplateAction(name, process)
            case "software_sources"    => SoftSourceAction(name, process)
            case _                     =>
              // Handle unknown processor types.
              println(f"Unknown action processor (${Utils
                  .toString(process("processor"))}) in $name")
              break(None)
          }
        }
      }).flatten

    // Return an ActionsFile containing the extracted actions.
    Some(ActionsFile(actions.toSeq))
  }
}
