package actions

import gagameos._
import overlord.Project
import overlord.Instances.{InstanceTrait, ProgramInstance, SoftwareInstance}
import java.nio.file.{Path, Paths}

// Represents an action to create symbolic links for software source files
case class SoftSourceAction(
    override val phase: Int, // Phase of the action (1 or 2)
    cpus: Seq[String], // List of CPU targets
    catalog_path: Path, // Base path for input files
    in: String, // Input file name pattern
    out: String
) // Output file name pattern
    extends Action() {
  // Executes the action for a given instance and parameters
  override def execute(
      instance: InstanceTrait,
      parameters: Map[String, Variant]
  ): Unit = {
    var ifn = in // Input file name
    var ofn = out // Output file name

    // Determine the CPU name based on the phase and parameters
    val cpuName =
      if (phase == 1) ""
      else if (cpus.nonEmpty) {
        val cpuName = Utils.toString(parameters("${cpuName}"))
        if (!cpus.contains(cpuName))
          return // Skip if CPU is not in the target list
        cpuName
      } else Utils.toString(parameters("${cpuName}"))

    // Replace placeholders in file names with parameter values
    for ((k, v) <- parameters) {
      ifn = ifn.replace(s"$k", v.toCString)
      ofn = ofn.replace(s"$k", v.toCString)
    }

    // Resolve the input file path
    val iPath = catalog_path.resolve(ifn)

    // Resolve the output file path based on the instance type
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

    // Ensure the output directory exists
    Utils.ensureDirectories(oPath.getParent)

    // Delete the existing file if it exists and create a symbolic link
    Utils.deleteFileIfExists(oPath)
    Utils.createSymbolicLink(iPath, oPath)
  }
}

object SoftSourceAction {
  private val cpuRegEx = "\\s*,\\s*".r // Regular expression to split CPU lists

  // Factory method to create SoftSourceAction instances from a process definition
  def apply(
      name: String,
      process: Map[String, Variant]
  ): Seq[SoftSourceAction] = {
    if (!process.contains("sources")) {
      println(s"SoftSourceAction process $name doesn't have a sources field")
      return Seq() // Return an empty sequence if no sources are defined
    }
    val srcs = Utils.toArray(process("sources")).map(Utils.toTable)

    // Per-process CPU target lists
    val allCpus = if (process.contains("cpus")) {
      val cpusString = Utils.toString(process("cpus"))
      if (cpusString == "_") Some(Seq()) // Empty list if "_" is specified
      else Some(cpuRegEx.split(cpusString).toSeq.map(_.toLowerCase()))
    } else None

    // Create actions for each source entry
    for (entry <- srcs.toIndexedSeq) yield {
      val (phase, cpus) =
        if (entry.contains("cpus")) {
          val cpusString = Utils.toString(entry("cpus"))
          if (cpusString == "_") (2, Seq()) // Phase 2 with no CPU targets
          else {
            val cpus = cpuRegEx.split(cpusString).map(_.toLowerCase())
            if (allCpus.isDefined) (2, cpus.intersect(allCpus.get).toSeq)
            else (2, cpus.toSeq)
          }
        } else if (allCpus.isDefined)
          (2, allCpus.get) // Phase 2 with global CPU targets
        else (1, Seq()) // Phase 1 with no CPU targets

      // Extract input and output file names
      val inFilename = Utils.toString(entry("in"))
      val outFilename = Utils.toString(entry("out"))

      // Create a SoftSourceAction instance
      SoftSourceAction(
        phase,
        cpus,
        Project.catalogPath,
        inFilename,
        outFilename
      )
    }
  }
}
