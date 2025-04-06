package actions

import gagameos._
import input.VerilogPort
import overlord.Hardware.{Port, WireDirection}
import overlord.Project
import overlord.Instances.{ChipInstance, InstanceTrait}
import scala.util.boundary, boundary.break

// Represents an action to read YAML-defined registers and associate them with a chip instance.
case class ReadYamlRegistersAction(name: String, filename: String)
    extends GatewareAction {

  // Defines the execution phase for this action.
  override val phase: Int = 2

  // Executes the action for a given instance and parameters.
  def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    // Check if the instance is a ChipInstance; otherwise, log a warning.
    if (!instance.isInstanceOf[ChipInstance]) {
      println(
        s"${instance.name} is not a chip but is being processed by a gateware action"
      )
    } else {
      // Cast the instance to ChipInstance and execute the action.
      execute(instance.asInstanceOf[ChipInstance], parameters)
    }
  }

  // Executes the action specifically for a ChipInstance.
  override def execute(
      instance: ChipInstance,
      parameters: Map[String, Variant]
  ): Unit = {
    // Resolve macros in the filename based on the instance context.
    val expandedName = Project.resolveInstanceMacros(instance, filename)
    // Parse the YAML file to retrieve register definitions.
    val registers =
      input.YamlRegistersParser(instance, expandedName, instance.name)
    // Append the parsed registers to the instance's register banks.
    instance.instanceRegisterBanks ++= registers
  }
}

// Companion object to create instances of ReadYamlRegistersAction.
object ReadYamlRegistersAction {
  // Factory method to create a ReadYamlRegistersAction from a process definition.
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Option[ReadYamlRegistersAction] = {
    // Ensure the process contains a "source" field; otherwise, log a warning and return None.
    if (!process.contains("source")) {
      println(s"Read Yaml Registers process $name doesn't have a source field")
      return None
    }

    // Extract the filename from the "source" field, handling different data types.
    val filename = process("source") match {
      case s: StringV => s.value
      case t: TableV  => Utils.toString(t.value("file"))
      case _          =>
        // Log an error if the "source" field is malformed and return None.
        println("Read Yaml Register source field is malformed")
        return None
    }

    // Return a new instance of ReadYamlRegistersAction.
    Some(ReadYamlRegistersAction(name, filename))
  }
}
