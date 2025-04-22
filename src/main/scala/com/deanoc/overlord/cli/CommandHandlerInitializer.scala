package com.deanoc.overlord.cli

import com.deanoc.overlord.cli.commands.CommandHandlerRegistry
import com.deanoc.overlord.utils.Logging

import com.deanoc.overlord.cli.commands.create.DefaultTemplatesCreateHandler
import com.deanoc.overlord.cli.commands.create.GccToolchainCreateHandler

import com.deanoc.overlord.cli.commands.project.ProjectCreateHandler
import com.deanoc.overlord.cli.commands.project.ProjectDefinitionListHandler
import com.deanoc.overlord.cli.commands.project.ProjectReportHandler
import com.deanoc.overlord.cli.commands.project.ProjectUpdateHandler

/**
 * Initializes command handlers during application startup
 */
object CommandHandlerInitializer extends Logging {
  /**
   * Initialize all command handlers
   * This should be called during application startup
   */
  def initialize(): Unit = {
    info("Initializing command handlers...")
    
    // Register create commands
    CommandHandlerRegistry.registerHandler("project", "create", ProjectCreateHandler)
    CommandHandlerRegistry.registerHandler("project", "definition-list", ProjectDefinitionListHandler)
    CommandHandlerRegistry.registerHandler("project", "update", ProjectUpdateHandler)
    CommandHandlerRegistry.registerHandler("project", "report", ProjectReportHandler)
    
    // Register generate commands
    CommandHandlerRegistry.registerHandler("create", "default-templates", DefaultTemplatesCreateHandler)
    CommandHandlerRegistry.registerHandler("create", "gcc-toolchain", GccToolchainCreateHandler)    
   
    info("Command handlers initialized")
  }
}
