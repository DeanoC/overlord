package com.deanoc.overlord.cli.commands.project

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.cli.commands.CommandHandler
import com.deanoc.overlord.Component

import java.nio.file.Paths
import scala.collection._
import com.deanoc.overlord.definitions.DefinitionType

object ProjectDefinitionListHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    config.options.get("project-file") match {
      case Some(filename: String) =>
        try {
          // Extract project name from filename
          val projectName =
            Paths.get(filename).getFileName.toString.split('.').head
          val boardName = config.boardName.getOrElse("unknown")

          // Use Component.fromTopLevelComponentFile to load the project
          val filePath = Paths.get(filename).toAbsolutePath
          val component = Component.fromTopLevelComponentFile(
            projectName,
            boardName,
            filePath
          )
          val container = component.getContainer
          val defcatalog = component.getCatalog
   
          println("\n# Definitions by Type")

          // Group definitions by definition type class name instead of instance
          // First create a map of class name to all unique definition types of that class
          val defTypesByClass = mutable.Map[String, mutable.Set[DefinitionType]]()
          
          // Collect all unique definition types by their class
          defcatalog.definitions.values.foreach { definition =>
            val typeName = definition.defType.ident(0)
            val defTypeSet = defTypesByClass.getOrElseUpdate(typeName, mutable.Set[DefinitionType]())
            defTypeSet += definition.defType
          }

          // Print each class and its definitions
          defTypesByClass.foreach { case (className, defTypes) =>
            println(s"## $className")
            
            // For each definition type of this class, print all definitions that use it
            defTypes.foreach { defType =>
              // Get all definitions that use this specific definition type
              val defsWithThisType = defcatalog.definitions.values.filter(_.defType == defType)
              defsWithThisType.foreach { definition =>
                println(s"  ${definition.defType.ident.mkString(".")}")
              }
            }
          }
          true
        } catch {
          case e: Exception =>
            error(s"Error generating definitions list: ${e.getMessage}")
            false
        }
      case _ =>
        error("Missing required project file argument")
        false
    }
  }
}
