package com.deanoc.overlord.cli.commands.project

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.cli.commands.CommandHandler
import com.deanoc.overlord.templates.TemplateManager
import com.deanoc.overlord.GlobalState

object ProjectCreateHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    val templateNameOpt = config.templateName
    val projectNameOpt = config.projectName

    GlobalState.allowWrites()

    (templateNameOpt, projectNameOpt) match {
      case (Some(templateName), Some(projectName)) =>
        // Check if any templates are available
        if (!TemplateManager.hasTemplates()) {
          info("No templates available")

          // Ask if the user wants to download standard templates
          val shouldDownload = if (config.yes) {
            info("Auto-downloading standard templates (-y/--yes specified)...")
            true
          } else {
            info("Waiting for user input about downloading templates")
            print(
              "No templates are installed. Would you like to download a standard set of templates from GitHub? (y/n): "
            )
            val response = scala.io.StdIn.readLine()
            info(s"User response for download: ${
                if (response == "y") "yes" else "no"
              }")
            response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
          }

          if (shouldDownload) {
            if (!TemplateManager.downloadStandardTemplates(config.yes)) {
              warn("Failed to download standard templates")
              return false
            }
          } else {
            info("Template download skipped")
            return false
          }
        }

        // Now try to create the project from the template
        TemplateManager.createFromTemplate(
          templateName,
          projectName,
          "." // Use current directory as output path
        )
      case (None, _) =>
        error("Missing required template name")
        false
      case (_, None) =>
        error("Missing required project name")
        false
    }
  }
}
