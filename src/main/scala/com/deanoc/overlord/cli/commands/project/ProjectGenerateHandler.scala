package com.deanoc.overlord.cli.commands.project

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.cli.commands.CommandHandler
import com.deanoc.overlord.templates.TemplateManager
import com.deanoc.overlord.GlobalState
import com.deanoc.overlord.Component
import java.nio.file.Paths

object ProjectGenerateHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    val projectNameOpt = config.projectName

    GlobalState.setProjectToGenerateMode()

    projectNameOpt match {
      case Some(projectPath) =>
        val fileName = Paths.get(projectPath).getFileName.toString.split('.').head
        info(
          s"Generating project '$fileName'..."
        )
  
        val boardName = config.boardName.getOrElse("unknown")
        val filePath = Paths.get(projectPath).toAbsolutePath

        val component = Component.fromTopLevelComponentFile(
          fileName,
          boardName,
          filePath
        )
        true
      case None =>
        error("Missing required project name")
        false
      }
  }
}
