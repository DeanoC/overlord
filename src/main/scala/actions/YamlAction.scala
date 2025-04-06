package actions

import gagameos._
import overlord.Project
import overlord.Instances.InstanceTrait

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
    val moddedOutPath = Project.outPath.resolve(
      Project.resolvePathMacros(instance, instance.name)
    )

    val dstAbsPath =
      moddedOutPath.resolve(Project.resolvePathMacros(instance, filename))
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
  def apply(name: String, process: Map[String, Variant]): Seq[YamlAction] = {
    // Validate the presence and type of the "parameters" field.
    if (!process.contains("parameters")) {
      println(s"Yaml process $name doesn't have a parameters field")
      return Seq()
    }
    if (!process("parameters").isInstanceOf[ArrayV]) {
      println(s"Yaml process $name parameters isn't an array")
      return Seq()
    }
    // Validate the presence and type of the "filename" field.
    if (!process.contains("filename")) {
      println(s"Yaml process $name doesn't have a filename field")
      return Seq()
    }
    if (!process("filename").isInstanceOf[StringV]) {
      println(s"Yaml process $name filename isn't a string")
      return Seq()
    }

    // Extract and convert the filename and parameters.
    val filename = Utils.toString(process("filename"))
    val parameters = Utils.toArray(process("parameters")).map(Utils.toString)

    // Return a sequence of YamlAction instances.
    Seq(YamlAction(parameters.toIndexedSeq, filename))
  }
}
