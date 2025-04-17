package com.deanoc.overlord.actions

import com.deanoc.overlord.utils._
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.instances.InstanceTrait

import scala.collection.mutable

// Represents an action that generates a YAML file based on provided parameters.
case class YamlAction(parameterKeys: Seq[String], filename: String)
    extends Action {

  // Defines the execution phase for this action.
  override val phase: Int = 1

  // Executes the action by writing parameters to a YAML file.
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    val sb = new mutable.StringBuilder()

    // Iterate over parameters and format them as YAML key-value pairs.
    for { (k, v) <- parameters }
      sb ++= (v match {
        case ArrayV(arr) =>
          s"$k:\n" + arr.map(item => s"  - ${formatValue(item)}").mkString("\n")
        case BigIntV(bigInt)   => s"$k: $bigInt\n"
        case BooleanV(boolean) => s"$k: $boolean\n"
        case IntV(int)         => s"$k: $int\n"
        case TableV(table) =>
          s"$k:\n" + table
            .map { case (key, value) =>
              s"  ${escapeString(key)}: ${formatValue(value)}"
            }
            .mkString("\n")
        case StringV(string) => s"$k: '${escapeString(string)}'\n"
        case DoubleV(dbl)    => s"$k: $dbl\n"
      })

    // Resolve the output path and ensure directories exist.
    val moddedOutPath = Overlord.outPath.resolve(
      Overlord.resolvePathMacros(instance, instance.name)
    )

    val dstAbsPath =
      moddedOutPath.resolve(Overlord.resolvePathMacros(instance, filename))
    Utils.ensureDirectories(dstAbsPath.getParent)

    // Write the formatted YAML content to the file.
    Utils.writeFile(dstAbsPath, sb.result())
  }

  // Helper method to format values for YAML.
  private def formatValue(value: Variant): String = value match {
    case ArrayV(arr) =>
      arr.map(item => s"- ${formatValue(item)}").mkString("\n")
    case TableV(table) =>
      table
        .map { case (key, value) =>
          s"${escapeString(key)}: ${formatValue(value)}"
        }
        .mkString("\n")
    case StringV(string) => s"'${escapeString(string)}'"
    case BigIntV(bigInt) => bigInt.toString
    case BooleanV(bool)  => bool.toString
    case IntV(int)       => int.toString
    case DoubleV(dbl)    => dbl.toString
  }

  // Helper method to escape strings for YAML.
  private def escapeString(input: String): String = {
    input.replace("'", "''")
  }
}

object YamlAction {
  // Factory method to create YamlAction instances from a process definition.
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Either[String, Seq[YamlAction]] = {
    if (!process.contains("parameters")) {
      Left(s"Yaml process $name doesn't have a parameters field")
    } else if (!process("parameters").isInstanceOf[ArrayV]) {
      Left(s"Yaml process $name parameters isn't an array")
    } else if (!process.contains("filename")) {
      Left(s"Yaml process $name doesn't have a filename field")
    } else if (!process("filename").isInstanceOf[StringV]) {
      Left(s"Yaml process $name filename isn't a string")
    } else {
      try {
        // Extract and convert the filename and parameters.
        val filename = Utils.toString(process("filename"))
        val parameters =
          Utils.toArray(process("parameters")).map(Utils.toString)

        // Return a sequence of YamlAction instances.
        Right(Seq(YamlAction(parameters.toIndexedSeq, filename)))
      } catch {
        case e: Exception =>
          Left(s"Error processing yaml in $name: ${e.getMessage}")
      }
    }
  }

  // Legacy method for backward compatibility
  def fromProcess(
      name: String,
      process: Map[String, Variant]
  ): Seq[YamlAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) =>
        println(errorMsg)
        Seq.empty
    }
  }
}
