package com.deanoc.overlord.cli

/** Configuration parsed from command line arguments */
case class Config(
  // Main command and subcommand
  command: Option[String] = None,
  subCommand: Option[String] = None,
  
  // Input file
  inFile: Option[String] = None,
  
  // Common flags
  yes: Boolean = false,
  noExit: Boolean = false,
  
  // Logging
  trace: Option[String] = None,
  debug: Option[String] = None,
  
  // Project options
  projectName: Option[String] = None,
  templateName: Option[String] = None,
  boardName: Option[String] = None,
  destination: Option[String] = None,
  
  // GCC toolchain options
  gccVersion: Option[String] = None,
  binutilsVersion: Option[String] = None,
  
  // Dynamic options map to handle all other options and arguments
  options: Map[String, String] = Map()
) {
  // Helper methods to access common options
  def getOption(name: String): Option[String] = options.get(name)
  
  def getRequiredOption(name: String): Either[String, String] = 
    options.get(name).toRight(s"Missing required argument: $name")
  
  // Pretty string representation for debugging
  override def toString: String = {
    val cmdStr = command.getOrElse("None")
    val subcmdStr = subCommand.getOrElse("None")
    val optsStr = options.mkString(",")
    s"Config(command=$cmdStr, subCommand=$subcmdStr, options=$optsStr)"
  }
}
