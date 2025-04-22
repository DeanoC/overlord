package com.deanoc.overlord.cli.commands.project

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.cli.commands.CommandHandler
import com.deanoc.overlord.templates.TemplateManager

object ProjectUpdateHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    val templateNameOpt = config.templateName
    val projectNameOpt = config.projectName

    (templateNameOpt, projectNameOpt) match {
      case (Some(templateName), Some(projectName)) =>
        info(
          s"TODO Updating project '$projectName' with template '$templateName'..."
        )
        true
      case (None, _) =>
        error("Missing required template name")
        false
      case (_, None) =>
        error("Missing required project name")
        false
    }
  }
}
