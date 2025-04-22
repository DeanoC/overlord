package com.deanoc.overlord.cli

object HelpTextManager {
  def getGlobalHelp(): String = {
    val sb = new StringBuilder
    sb.append("USAGE:\n  overlord <command> [subcommand] [options]\n\n")
    sb.append("COMMANDS:\n")
    val commands = CommandLineParser.getAllCommands
    val maxLen = commands.map(_._1.length).maxOption.getOrElse(0)
    commands.foreach { case (name, desc) =>
      sb.append(f"  ${name.padTo(maxLen, ' ')}  $desc\n")
    }
    sb.append("\n")
    // Show common/global options
    val commonOpts = CommandLineParser.commonOptions
    if (commonOpts.nonEmpty) {
      sb.append("OPTIONS:\n")
      commonOpts.foreach { o =>
        val short = o.short.map(s => s"-$s, ").getOrElse("")
        sb.append(s"  $short--${o.name} ${o.description}")
        if (o.required) sb.append(" (required)")
        sb.append("\n")
      }
      sb.append("\n")
    }
    sb.append("Run 'overlord help <command>' for more information on a specific command.\n")
    sb.toString
  }

  def getCommandHelp(command: String): String = {
    val subcommands = CommandLineParser.getSubcommandsFor(command)
    val cmdMetaOpt = CommandLineParser.allCommandMetas.find(_.name == command)
    cmdMetaOpt match {
      case Some(cmdMeta) =>
        val sb = new StringBuilder
        sb.append(s"Command: ${cmdMeta.name}\n\n")
        val desc = cmdMeta.longDescription.getOrElse(cmdMeta.description)
        sb.append("DESCRIPTION:\n  ").append(desc).append("\n\n")
        sb.append("USAGE:\n  overlord ").append(cmdMeta.name).append(" [subcommand] [options]\n\n")
        if (cmdMeta.subcommands.nonEmpty) {
          sb.append("SUBCOMMANDS:\n")
          cmdMeta.subcommands.foreach { sub =>
            sb.append(s"  ${sub.name}  ${sub.description}\n")
          }
          sb.append("\n")
        }
        // Show common/global options (if not already shown above)
        if (cmdMeta.subcommands.isEmpty && CommandLineParser.commonOptions.nonEmpty) {
          sb.append("OPTIONS:\n")
          CommandLineParser.commonOptions.foreach { o =>
            val short = o.short.map(s => s"-$s, ").getOrElse("")
            sb.append(s"  $short--${o.name} ${o.description}")
            if (o.required) sb.append(" (required)")
            sb.append("\n")
          }
          sb.append("\n")
        }
        if (cmdMeta.examples.nonEmpty) {
          sb.append("EXAMPLES:\n")
          cmdMeta.examples.foreach { ex =>
            sb.append(s"  $ex\n")
          }
          sb.append("\n")
        }
        sb.toString
      case None =>
        s"No help available for command: $command"
    }
  }

  def getSubcommandHelp(command: String, subcommand: String): String = {
    val (args, opts) = CommandLineParser.getArgsAndOptions(command, Some(subcommand))
    val cmdMetaOpt = CommandLineParser.allCommandMetas.find(_.name == command)
    val subMetaOpt = cmdMetaOpt.flatMap(_.subcommands.find(_.name == subcommand))
    (cmdMetaOpt, subMetaOpt) match {
      case (Some(cmdMeta), Some(subMeta)) =>
        val sb = new StringBuilder
        sb.append(s"Command: ${cmdMeta.name} ${subMeta.name}\n\n")
        val desc = subMeta.longDescription.orElse(cmdMeta.longDescription).getOrElse(subMeta.description)
        sb.append("DESCRIPTION:\n  ").append(desc).append("\n\n")
        sb.append("USAGE:\n  overlord ").append(cmdMeta.name).append(" ").append(subMeta.name)
        if (args.nonEmpty) {
          sb.append(" ")
          sb.append(args.map(a => s"<${a.name}>").mkString(" "))
        }
        sb.append(" [options]\n\n")
        if (args.nonEmpty) {
          sb.append("ARGUMENTS:\n")
          args.foreach { a =>
            sb.append(s"  <${a.name}>  ${a.description}")
            if (a.required) sb.append(" (required)")
            sb.append("\n")
          }
          sb.append("\n")
        }
        // Show options: subcommand + command + common/global
        val allOpts = (opts ++ cmdMeta.options ++ CommandLineParser.commonOptions)
          .groupBy(_.name)
          .map(_._2.head)
          .toList
        if (allOpts.nonEmpty) {
          sb.append("OPTIONS:\n")
          allOpts.foreach { o =>
            val short = o.short.map(s => s"-$s, ").getOrElse("")
            sb.append(s"  $short--${o.name} ${o.description}")
            if (o.required) sb.append(" (required)")
            sb.append("\n")
          }
          sb.append("\n")
        }
        if (subMeta.examples.nonEmpty) {
          sb.append("EXAMPLES:\n")
          subMeta.examples.foreach { ex =>
            sb.append(s"  $ex\n")
          }
          sb.append("\n")
        }
        sb.toString
      case _ =>
        s"No help available for command: $command $subcommand"
    }
  }

  def getFocusedUsage(config: CliConfig): String = {
    (config.command, config.subCommand) match {
      case (Some(cmd), Some(sub)) =>
        if (CommandLineParser.commandExists(cmd)) {
          if (CommandLineParser.subcommandExists(cmd, sub)) {
            getSubcommandHelp(cmd, sub)
          } else {
            getInvalidSubcommandHelp(cmd, sub)
          }
        } else {
          getInvalidCommandHelp(cmd)
        }
      case (Some(cmd), None) =>
        if (CommandLineParser.commandExists(cmd)) {
          getCommandHelp(cmd)
        } else {
          getInvalidCommandHelp(cmd)
        }
      case _ =>
        getGlobalHelp()
    }
  }
  
  // Generate help text for an invalid command
  def getInvalidCommandHelp(command: String): String = {
    val sb = new StringBuilder
    sb.append(s"Error: Unknown command '$command'\n\n")
    sb.append("Available commands:\n")
    CommandLineParser.getAllCommands.foreach { case (name, desc) =>
      sb.append(s"  $name  $desc\n")
    }
    sb.append("\nRun 'overlord help <command>' for more information on a specific command.\n")
    sb.toString
  }
  
  // Generate help text for an invalid subcommand
  def getInvalidSubcommandHelp(command: String, subcommand: String): String = {
    val sb = new StringBuilder
    sb.append(s"Error: Unknown subcommand '$subcommand' for command '$command'\n\n")
    
    // Check if there's a similar subcommand (typo correction)
    val suggestedSubcommand = CommandLineParser.findClosestSubcommand(command, subcommand)
    suggestedSubcommand.foreach { suggestion =>
      sb.append(s"Did you mean: $suggestion?\n\n")
    }
    
    // List available subcommands
    val subcommands = CommandLineParser.getSubcommandsFor(command)
    if (subcommands.nonEmpty) {
      sb.append(s"Available subcommands for '$command':\n")
      subcommands.foreach { case (name, desc) =>
        sb.append(s"  $name  $desc\n")
      }
    }
    
    sb.append(s"\nRun 'overlord help $command' for more information.\n")
    sb.toString
  }
}