package com.deanoc.overlord.cli

import java.nio.file.Path
import scala.collection.mutable

/** Configuration class for the Overlord CLI. Supports a hierarchical command
  * structure with primary and secondary commands.
  */
case class Config(
    // Primary command (create, generate, clean, update, template)
    command: Option[String] = None,

    // Secondary command (project, from-template, test, report, svd, catalog, etc.)
    subCommand: Option[String] = None,

    // Common options
    board: Option[String] = None,
    nostdresources: Boolean = false,
    nostdprefabs: Boolean = false,
    resources: Option[String] = None,
    yes: Boolean = false,
    noexit: Boolean = false,
    trace: Option[String] = None,
    debug: Option[String] = None,

    // Command-specific options
    infile: Option[String] = None,
    instance: Option[String] = None,
    templateName: Option[String] = None,
    projectName: Option[String] = None,
    stdresource: Option[String] = None,

    // Additional options for git/GitHub integration
    gitUrl: Option[String] = None,
    branch: Option[String] = None,
    ownerRepo: Option[String] = None,
    ref: Option[String] = None,

    // Generic options map for additional parameters
    options: mutable.Map[String, Any] = mutable.Map.empty
)
