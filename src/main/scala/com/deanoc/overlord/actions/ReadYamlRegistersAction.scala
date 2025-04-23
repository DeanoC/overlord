package com.deanoc.overlord.actions

import com.deanoc.overlord.utils._
import com.deanoc.overlord.input.{VerilogPort, YamlRegistersParser}
import com.deanoc.overlord.hardware.Port
import com.deanoc.overlord.config.WireDirection
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.instances.{HardwareInstance, InstanceTrait}
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
    if (!instance.isInstanceOf[HardwareInstance]) {
      println(
        s"${instance.name} is not a chip but is being processed by a gateware action"
      )
    } else {
      // Cast the instance to ChipInstance and execute the action.
      execute(instance.asInstanceOf[HardwareInstance], parameters)
    }
  }

  // Executes the action specifically for a ChipInstance.
  override def execute(
      instance: HardwareInstance,
      parameters: Map[String, Variant]
  ): Unit = {
    // Resolve macros in the filename based on the instance context.
    val expandedName = Overlord.resolveInstanceMacros(instance, filename)
    // Parse the YAML file to retrieve register definitions.
    YamlRegistersParser(instance, expandedName, instance.name) match {
      case Right(registers) =>
        // Append the parsed registers to the instance's register banks.
        instance.instanceRegisterBanks ++= registers
      case Left(error) =>
        println(s"Error in ${instance.name}: $error")
    }
  }
}

object ReadYamlRegistersAction {
  // Factory method to create a ReadYamlRegistersAction from a process definition.
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Either[String, Seq[ReadYamlRegistersAction]] = {
    // Ensure the process contains a "source" field
    if (!process.contains("source")) {
      Left(s"Read Yaml Registers process $name doesn't have a source field")
    } else {
      try {
        // Extract the filename from the "source" field, handling different data types.
        val filenameEither = process("source") match {
          case s: StringV => Right(s.value)
          case t: TableV =>
            if (!t.value.contains("file")) {
              Left(
                s"Read Yaml Registers process $name has a table without a file field"
              )
            } else {
              Right(Utils.toString(t.value("file")))
            }
          case other =>
            Left(
              s"Read Yaml Registers source field is malformed: ${other.getClass.getSimpleName}"
            )
        }

        filenameEither.flatMap { filename =>
          if (filename.isEmpty) {
            Left(s"Read Yaml Registers process $name has an empty filename")
          } else {
            Right(Seq(ReadYamlRegistersAction(name, filename)))
          }
        }
      } catch {
        case e: Exception =>
          Left(
            s"Error processing Read Yaml Registers in $name: ${e.getMessage}"
          )
      }
    }
  }

  // Legacy method for backward compatibility
  def fromProcess(
      name: String,
      process: Map[String, Variant]
  ): Seq[ReadYamlRegistersAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) =>
        println(errorMsg)
        Seq.empty
    }
  }
}
