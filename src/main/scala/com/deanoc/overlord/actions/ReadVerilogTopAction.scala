package com.deanoc.overlord.actions

import scala.util.boundary
import boundary.break

import com.deanoc.overlord.utils.{Utils, Variant, StringV, TableV, Logging}
import com.deanoc.overlord.input.{
  VerilogParameterKey,
  VerilogPort,
  VerilogModuleParser
}
import com.deanoc.overlord.hardware.{Port, WireDirection}
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.instances.{ChipInstance, InstanceTrait}

case class ReadVerilogTopAction(filename: String)
    extends GatewareAction
    with Logging {

  override val phase: Int = 2

  // Executes the action on a given instance, ensuring it is a ChipInstance
  def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    if (!instance.isInstanceOf[ChipInstance]) {
      warn(
        s"${instance.name} is not a chip but is being processed by a gateware action"
      )
    } else {
      execute(instance.asInstanceOf[ChipInstance], parameters)
    }
  }

  // Executes the action specifically for ChipInstance, parsing Verilog modules
  override def execute(
      instance: ChipInstance,
      parameters: Map[String, Variant]
  ): Unit = {
    import scala.util.boundary, boundary.break

    // Resolves the filename with macros and parses Verilog modules
    val expandedName = Overlord.resolveInstanceMacros(instance, filename)
    debug(
      s"Parsing Verilog module from ${expandedName} for instance ${instance.name}"
    )

    VerilogModuleParser(
      Overlord.tryPaths(instance, expandedName),
      instance.name
    ) match {
      case Right(parsedModules) =>
        boundary {
          // Log all available module names for debugging purposes

          // Finds the module matching the instance name or filename
          val module = parsedModules.find(_.name == instance.name).getOrElse {
            debug(
              s"Module name '${instance.name}' not found, trying filename-based match"
            )
            val filenameBasedModuleName =
              expandedName.split("/").last.replace(".v", "")
            parsedModules
              .find(_.name == filenameBasedModuleName)
              .getOrElse {
                warn(
                  s"No matching module found for ${instance.name}. Tried instance name and '$filenameBasedModuleName'"
                )
                debug(
                  s"Available modules in $expandedName: [${parsedModules.map(_.name).mkString(", ")}]"
                )
                break(())
              }
          }

          debug(s"Found matching module: ${module.name}")

          // Updates the instance with module details
          instance.moduleName = module.name
          val ports = module.module_boundary.collect { case p: VerilogPort =>
            p
          }
          val parameterKeys = module.module_boundary.collect {
            case p: VerilogParameterKey => p
          }

          // Log detected ports and parameters
          debug(s"Detected ${ports.size} ports for ${instance.name}:")
          ports.foreach(p =>
            debug(
              s"  - Port: ${p.name}, Direction: ${p.direction}, Bits: ${p.bits}, Width: ${p.knownWidth}"
            )
          )

          debug(
            s"Detected ${parameterKeys.size} parameters for ${instance.name}:"
          )
          parameterKeys.foreach(p => debug(s"  - Parameter: ${p.parameter}"))

          ports.foreach(p =>
            instance.mergePort(
              p.name,
              Port(p.name, p.bits, WireDirection(p.direction), p.knownWidth)
            )
          )
          parameterKeys.foreach(p => instance.mergeParameterKey(p.parameter))
        }

      case Left(err) =>
        error(s"Error parsing Verilog module for ${instance.name}: $err")
    }
  }
}

object ReadVerilogTopAction extends Logging {
  // Factory method to create a ReadVerilogTopAction if the process contains a source field
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Either[String, Seq[ReadVerilogTopAction]] = {
    if (!process.contains("source")) {
      Left(s"Read Verilog Top process $name doesn't have a source field")
    } else {
      try {
        // Extracts the filename from the process map
        val filenameEither = process("source") match {
          case s: StringV => Right(s.value)
          case t: TableV =>
            if (!t.value.contains("file")) {
              Left(
                s"Read Verilog Top process $name has a table without a file field"
              )
            } else {
              Right(Utils.toString(t.value("file")))
            }
          case other =>
            Left(
              s"Read Verilog Top process $name source is an invalid type: ${other.getClass.getSimpleName}"
            )
        }
        filenameEither.flatMap { filename =>
          if (filename.isEmpty) {
            Left(s"Read Verilog Top process $name has an empty filename")
          } else {
            Right(Seq(ReadVerilogTopAction(filename)))
          }
        }
      } catch {
        case e: Exception =>
          Left(s"Error processing Read Verilog Top in $name: ${e.getMessage}")
      }
    }
  }

  // Legacy method for backward compatibility
  def fromProcess(
      name: String,
      process: Map[String, Variant]
  ): Seq[ReadVerilogTopAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) =>
        error(errorMsg)
        Seq.empty
    }
  }
}
