package com.deanoc.overlord.actions

import java.nio.file.{Path, Paths}

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.instances.{
  InstanceTrait,
  ProgramInstance,
  SoftwareInstance
}

case class TemplateAction(
    override val phase: Int,
    cpus: Seq[String],
    catalog_path: java.nio.file.Path,
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

        Overlord.outPath
          .resolve(folder)
          .resolve(si.name.replace('.', '_'))
          .resolve(ofn)
      case _ =>
        Overlord.outPath.resolve(ofn)
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
  ): Either[String, Seq[TemplateAction]] = {
    // Check if the process contains a "sources" field
    if (!process.contains("sources")) {
      Left(s"Template process $name doesn't have a sources field")
    } else {

      try {
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

        // Check for required fields in all entries first
        val missingFields = srcs.zipWithIndex.collectFirst {
          case (entry, idx) if !entry.contains("in") =>
            Left(
              s"Template entry #${idx + 1} in $name is missing the 'in' field"
            )
          case (entry, idx) if !entry.contains("out") =>
            Left(
              s"Template entry #${idx + 1} in $name is missing the 'out' field"
            )
        }

        missingFields match {
          case Some(error) => error
          case None        =>
            // Create TemplateAction instances for each valid entry
            val actions = for (entry <- srcs.toIndexedSeq) yield {
              // Determine the phase and CPUs for the current entry
              val phaseAndCpus: (Int, Seq[String]) = {
                if (entry.contains("cpus")) {
                  val cpusString = Utils.toString(entry("cpus"))
                  if (cpusString == "_") (2, Seq()) // Special case for "_"
                  else {
                    val cpusList = cpuRegEx.split(cpusString)
                    if (all_cpus.isDefined)
                      (
                        2,
                        cpusList
                          .intersect(all_cpus.get)
                          .toSeq
                          .map(_.toLowerCase())
                      ) // Filter CPUs
                    else (2, cpusList.toSeq)
                  }
                } else if (all_cpus.isDefined)
                  (2, all_cpus.get) // Use global CPUs if defined
                else (1, Seq()) // Default to phase 1 with no CPUs
              }
              // Extract phase and CPUs from the tuple
              val phase = phaseAndCpus._1
              val cpus = phaseAndCpus._2

              // Extract input and output file names from the entry
              val inFilename = Utils.toString(entry("in"))
              val outFilename = Utils.toString(entry("out"))

              // Create a TemplateAction instance
              TemplateAction(
                phase,
                cpus,
                Overlord.catalogPath,
                inFilename,
                outFilename
              )
            }
            Right(actions)
        }
      } catch {
        case e: Exception =>
          Left(s"Error processing template in $name: ${e.getMessage}")
      }
    }
  }

  // Legacy method for backward compatibility
  def fromProcess(
      name: String,
      process: Map[String, Variant]
  ): Seq[TemplateAction] = {
    apply(name, process) match {
      case Right(actions) => actions
      case Left(errorMsg) =>
        println(errorMsg)
        Seq.empty
    }
  }
}
