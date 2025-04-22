package com.deanoc.overlord.cli.commands.create

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.cli.commands.CommandHandler
import com.deanoc.overlord.templates.TemplateManager

object DefaultTemplatesCreateHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    info("Downloading standard templates...")

    val templates = TemplateManager.listAvailableTemplates()
    if (templates.nonEmpty) {
      info(
        "Some templates are already installed. The following templates are available:"
      )
      templates.foreach { template =>
        info(s"  $template")
      }

      val shouldDownload = if (config.yes) {
        info("Auto-downloading standard templates (-y/--yes specified)...")
        true
      } else {
        print(
          "Do you want to download or update the standard templates? (y/n): "
        )
        val response = scala.io.StdIn.readLine()
        response != null && (response.trim.toLowerCase == "y" || response.trim.toLowerCase == "yes")
      }

      if (shouldDownload) {
        val result = TemplateManager.downloadStandardTemplates(config.yes)
        if (result) {
          info("Standard templates have been downloaded successfully.")
          val updatedTemplates = TemplateManager.listAvailableTemplates()
          info("Available templates:")
          updatedTemplates.foreach { template =>
            info(s"  $template")
          }
          return true
        } else {
          error("Failed to download some standard templates.")
          return false
        }
      } else {
        info("Template download skipped.")
        return true
      }
    } else {
      // No templates installed yet, proceed with download
      val result = TemplateManager.downloadStandardTemplates(config.yes)
      if (result) {
        info("Standard templates have been downloaded successfully.")
        val updatedTemplates = TemplateManager.listAvailableTemplates()
        info("Available templates:")
        updatedTemplates.foreach { template =>
          info(s"  $template")
        }
        return true
      } else {
        error("Failed to download standard templates.")
        return false
      }
    }
  }
}
