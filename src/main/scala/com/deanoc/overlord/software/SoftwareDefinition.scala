package com.deanoc.overlord.software

import com.deanoc.overlord.actions.ActionsFile
import com.deanoc.overlord.utils.{StringV, Utils, Variant}
import com.deanoc.overlord.{DefinitionType, Project, SoftwareDefinitionTrait}

import java.nio.file.Path

case class SoftwareDefinition(
    defType: DefinitionType,
    sourcePath: Path,
    attributes: Map[String, Variant],
    parameters: Map[String, Variant],
    dependencies: Seq[String],
    actionsFilePath: Path,
    actionsFile: ActionsFile
) extends SoftwareDefinitionTrait {}

object SoftwareDefinition {
  def apply(
      table: Map[String, Variant],
      path: Path
  ): Option[SoftwareDefinition] = {
    if (!table.contains("type")) {
      return None
    }

    val defTypeName = Utils.toString(table("type"))

    val software = if (!table.contains("software")) {
      val name = defTypeName.split('.')
      s"${name.last}/${name.last}.yaml"
    } else {
      Utils.toString(table("software"))
    }
    val name = if (!table.contains("name")) {
      val name = defTypeName.split('.')
      s"${name.last}"
    } else {
      Utils.toString(table("name"))
    }

    val dependencies: Seq[String] = if (table.contains("depends")) {
      val depends = Utils.toArray(table("depends"))
      depends.map(Utils.toString).toSeq
    } else {
      Seq()
    }

    val attribs = table.filter(a =>
      a._1 match {
        case "type" | "software" | "name" | "depends" => false
        case _                                        => true
      }
    ) ++ Map[String, Variant]("name" -> StringV(name))

    Some(
      SoftwareDefinition(
        DefinitionType(defTypeName),
        path,
        attribs,
        defTypeName,
        dependencies,
        path.resolve(software)
      ).get
    )
  }

  def apply(
      defType: DefinitionType,
      path: Path,
      attributes: Map[String, Variant],
      name: String,
      dependencies: Seq[String],
      softwarePath: Path
  ): Option[SoftwareDefinition] = {
    Project.pushCatalogPath(softwarePath)
    val result = parse(
      defType,
      path,
      attributes,
      name,
      dependencies,
      softwarePath,
      Utils.readYaml(softwarePath)
    )
    Project.popCatalogPath()
    result
  }

  private def parse(
      defType: DefinitionType,
      path: Path,
      attributes: Map[String, Variant],
      name: String,
      dependencies: Seq[String],
      softwarePath: Path,
      parsed: Map[String, Variant]
  ): Option[SoftwareDefinition] = {
    val actionsFile = ActionsFile.createActionsFile(name, parsed)

    if (actionsFile.isEmpty) {
      println(s"Software actions file $name invalid\n")
      return None
    }

    val parameters =
      if (parsed.contains("parameters"))
        Utils.toTable(parsed("parameters"))
      else Map[String, Variant]()

    Some(
      SoftwareDefinition(
        defType,
        path,
        attributes,
        parameters,
        dependencies,
        softwarePath,
        actionsFile.get
      )
    )
  }

}
