package com.deanoc.overlord.cli.commands

import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.cli.CliConfig
import scala.collection.mutable

/**
 * Registry for command handlers in the application.
 * 
 * This object maintains a registry of command handlers that can be looked up
 * by command and subcommand. It provides methods to register handlers and
 * retrieve them when needed for command execution.
 */
object CommandHandlerRegistry extends Logging {
  // Maps command -> subcommand -> handler
  private val handlers: mutable.Map[String, mutable.Map[String, CommandHandler]] = mutable.Map.empty

  /**
   * Register a command handler for a specific command and subcommand combination.
   *
   * @param command The primary command (e.g., "project", "create")
   * @param subcommand The secondary command (e.g., "create", "update", "report")
   * @param handler The command handler to register
   */
  def registerHandler(command: String, subcommand: String, handler: CommandHandler): Unit = {
    if (!handlers.contains(command)) {
      handlers(command) = mutable.Map.empty
    }
    
    handlers(command)(subcommand) = handler
    debug(s"Registered handler for command '$command $subcommand': ${handler.getClass.getSimpleName}")
  }

  /**
   * Check if a handler exists for the given command and subcommand.
   *
   * @param command The primary command
   * @param subcommand The secondary command
   * @return true if a handler exists, false otherwise
   */
  def handlerExists(command: String, subcommand: String): Boolean = {
    handlers.get(command).exists(_.contains(subcommand))
  }

  /**
   * Get the handler for the specified command and subcommand.
   *
   * @param command The primary command
   * @param subcommand The secondary command
   * @return Some(handler) if found, None otherwise
   */
  def getHandler(command: String, subcommand: String): Option[CommandHandler] = {
    for {
      subcommandMap <- handlers.get(command)
      handler <- subcommandMap.get(subcommand)
    } yield handler
  }

  /**
   * Get all registered commands.
   *
   * @return Set of registered command names
   */
  def getCommands: Set[String] = handlers.keySet.toSet

  /**
   * Get all subcommands for a specific command.
   *
   * @param command The primary command
   * @return Set of subcommand names for the given command
   */
  def getSubcommands(command: String): Set[String] = {
    handlers.get(command).map(_.keySet.toSet).getOrElse(Set.empty)
  }

  /**
   * Clear all registered handlers. Primarily used for testing.
   */
  def clear(): Unit = {
    handlers.clear()
    debug("Command handler registry cleared")
  }
}
