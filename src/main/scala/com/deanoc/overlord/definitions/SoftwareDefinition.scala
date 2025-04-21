package com.deanoc.overlord.definitions

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.{StringV, Utils, Variant}
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.definitions.DefinitionType
import com.deanoc.overlord.definitions.SoftwareDefinitionTrait
import com.deanoc.overlord.config.DefinitionConfig

import java.nio.file.Path

case class SoftwareDefinition(
    defType: DefinitionType,
    sourcePath: Path,
    attributes: Map[String, Variant],
    parameters: Map[String, Variant],
    dependencies: Seq[String],
    actionsFilePath: Path,
    actionsFile: ActionsFile,
    config: DefinitionConfig // Added config field
) extends SoftwareDefinitionTrait {}

object SoftwareDefinition {
  def apply(
      defType: DefinitionType,
      config: DefinitionConfig,
      path: Path
  ): Either[String, SoftwareDefinitionTrait] = {

    val attributes: Map[String, Any] = config.attributes

    val configMap: Map[String, Variant] = attributes.map { case (k, v) =>
      k -> Utils.toVariant(v) // Convert Any to Variant
    }

    // Extract the name from the defType or config
    val defTypeParts = defType.ident
    val name = if (!configMap.contains("name")) {
      // Use the last part of the identifier sequence
      defTypeParts.lastOption.getOrElse("unknown")
    } else {
      Utils.toString(configMap("name"))
    }

    // Determine the software path
    val software = if (!configMap.contains("software")) {
      // Use the last part of the identifier sequence
      val lastPart = defTypeParts.lastOption.getOrElse("unknown")
      s"$lastPart/$lastPart.yaml"
    } else {
      Utils.toString(configMap("software"))
    }

    // Extract dependencies
    val dependencies: Seq[String] = if (configMap.contains("depends")) {
      val depends = Utils.toArray(configMap("depends"))
      depends.map(Utils.toString).toSeq
    } else {
      Seq()
    }

    // Filter attributes
    val attribs = configMap.filter(a =>
      a._1 match {
        case "type" | "software" | "name" | "depends" => false
        case _                                        => true
      }
    ) ++ Map[String, Variant]("name" -> StringV(name))

    // Resolve the software path and create the definition
    val softwarePath = path.resolve(software)
    
    // Push catalog path, read YAML, and parse
    Overlord.pushCatalogPath(softwarePath)
    val result = Left("Software definition TODO")
    //    val result = Utils.loadAndParseYamlFile[GatewareDefinition](Overlord.catalogPath.resolve(fileNameAlone))
/*    
    // Create actions file
    val actionsFile = ActionsFile.createActionsFile(name, parsed)
    
    if (actionsFile.isEmpty) {
      Overlord.popCatalogPath()
      return Left(s"Software actions file $name invalid")
    }
    
    // Extract parameters
    val parameters =
      if (parsed.contains("parameters"))
        Utils.toTable(parsed("parameters"))
      else Map[String, Variant]()
    
    val result = Right(
      SoftwareDefinition(
        defType,
        path,
        attribs,
        parameters,
        dependencies,
        softwarePath,
        actionsFile.get,
        config
      )
    )
  */  
    Overlord.popCatalogPath()
    result
  }
}
