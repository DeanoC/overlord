package com.deanoc.overlord.cli.commands

import com.deanoc.overlord.cli.CliConfig
import com.deanoc.overlord.utils.Logging

/**
 * Trait defining the interface for command handlers in the application.
 * 
 * Command handlers are responsible for executing specific commands based on the
 * parsed command-line configuration. Each command/subcommand combination has its
 * own handler implementation that contains the business logic for that specific command.
 */
trait CommandHandler extends Logging {
  /**
   * Execute the command based on the provided configuration.
   *
   * @param config The parsed command line configuration containing the command, subcommand,
   *               and any options or arguments needed to execute the command
   * @return true if the command executed successfully, false otherwise
   */
  def execute(config: CliConfig): Boolean
  /* TODO refactor the help to use this
  /**
   * Get a description of the command. Used for help text and documentation.
   *
   * @return A string describing what the command does
   */
  def getDescription: String
  
  /**
   * Get the usage examples for this command. Used for help text.
   *
   * @return A list of example usages for this command
   */
  def getUsageExamples: List[String] = List.empty

*/
}
