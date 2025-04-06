package actions

import gagameos._
import overlord.Project
import overlord.Instances.{InstanceTrait, ProgramInstance, SoftwareInstance}
import java.nio.file.{Path, Paths}

case class TemplateAction(
    override val phase: Int,
    cpus: Seq[String],
    catalog_path: Path,
    in: String,
    out: String
) extends Action() {
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    // Initialize input and output file names from the provided parameters
    var ifn = in
    var ofn = out

    // Determine the CPU name based on the phase and provided parameters
    val cpuName =
      if (phase == 1) "" // Phase 1 does not use a specific CPU
      else if (cpus.nonEmpty) {
        // Extract the CPU name from parameters and check if it's valid
        val cpuName = Utils.toString(parameters("${cpuName}"))
        if (!cpus.contains(cpuName))
          return // Exit if the CPU is not in the allowed list
        cpuName
      } else
        Utils.toString(
          parameters("${cpuName}")
        ) // Default to the CPU name in parameters

    // Replace placeholders in input and output file names with parameter values
    for ((k, v) <- parameters) {
      ifn = ifn.replace(
        s"$k",
        v.toCString
      ) // Replace placeholders in the input file name
      ofn = ofn.replace(
        s"$k",
        v.toCString
      ) // Replace placeholders in the output file name
    }

    // Resolve the input file path relative to the catalog path
    val iPath = catalog_path.resolve(ifn)

    // Read the content of the input file
    val source = Utils.readFile(iPath)
    if (source.isEmpty) {
      // Print an error message if the input file is not found
      println(f"Template source file $iPath not found%n")
      return
    }

    var sourceString =
      source.get // Get the content of the input file as a string

    // Replace placeholders in the file content with parameter values
    for ((k, v) <- parameters) {
      sourceString = sourceString.replace(s"$k", v.toCString)
    }

    val oPath = instance match {
      case si: SoftwareInstance =>
        val folder = if (instance.isInstanceOf[ProgramInstance]) {
          if (cpuName.nonEmpty) s"${si.folder}_$cpuName"
          else si.folder
        } else si.folder

        Project.outPath
          .resolve(folder)
          .resolve(si.name.replace('.', '_'))
          .resolve(ofn)
      case _ =>
        Project.outPath.resolve(ofn)
    }
    Utils.ensureDirectories(oPath.getParent)
    Utils.writeFile(oPath, sourceString)
  }
}

object TemplateAction {
  // Regular expression to split CPU strings by commas with optional spaces
  private val cpuRegEx = "\\s*,\\s*".r

  // Factory method to create a sequence of TemplateAction instances
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Seq[TemplateAction] = {
    // Check if the process contains a "sources" field
    if (!process.contains("sources")) {
      println(s"Template process $name doesn't have a sources field")
      return Seq() // Return an empty sequence if "sources" is missing
    }

    // Convert the "sources" field into an array of tables
    val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

    // Parse the "cpus" field if it exists
    val all_cpus = if (process.contains("cpus")) {
      val cpusString = Utils.toString(process("cpus"))
      if (cpusString == "_") Some(Seq()) // Special case for "_"
      else
        Some(
          cpuRegEx.split(cpusString).toSeq
        ) // Split CPU string into a sequence
    } else None

    // Iterate over each source entry to create TemplateAction instances
    for (entry <- srcs.toIndexedSeq) yield {
      // Determine the phase and CPUs for the current entry
      val (phase, cpus) =
        if (entry.contains("cpus")) {
          val cpusString = Utils.toString(entry("cpus"))
          if (cpusString == "_") (2, Seq()) // Special case for "_"
          else {
            val cpus = cpuRegEx.split(cpusString)
            if (all_cpus.isDefined)
              (
                2,
                cpus.intersect(all_cpus.get).toSeq.map(_.toLowerCase())
              ) // Filter CPUs
            else (2, cpus.toSeq)
          }
        } else if (all_cpus.isDefined)
          (2, all_cpus.get) // Use global CPUs if defined
        else (1, Seq()) // Default to phase 1 with no CPUs

      // Extract input and output file names from the entry
      val inFilename = Utils.toString(entry("in"))
      val outFilename = Utils.toString(entry("out"))

      // Create a TemplateAction instance
      TemplateAction(phase, cpus, Project.catalogPath, inFilename, outFilename)
    }
  }
}
