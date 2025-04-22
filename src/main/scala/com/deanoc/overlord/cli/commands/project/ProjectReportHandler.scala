package com.deanoc.overlord.cli.commands.project

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.cli.commands.CommandHandler
import com.deanoc.overlord.cli.CommandExecutor
import com.deanoc.overlord.output

object ProjectReportHandler extends CommandHandler {
  override def execute(config: CliConfig): Boolean = {
    config.inFile match {
      case Some(filename) =>
        try {
          val game = CommandExecutor.loadProject(config, filename)
          if (game != null) {
            output.Report(game)
            true
          } else {
            false
          }
        } catch {
          case e: Exception =>
            error(s"Error generating report: ${e.getMessage}")
            false
        }
      case None =>
        error("Missing required input file")
        false
    }
  }
}
