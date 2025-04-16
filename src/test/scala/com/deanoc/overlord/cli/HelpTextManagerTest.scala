package com.deanoc.overlord.cli

import org.scalatest.flatspec.AnyFlatSpec
import org.scalatest.matchers.should.Matchers

/**
 * Test for HelpTextManager to ensure it produces help text with the expected
 * sections and content.
 */
class HelpTextManagerTest extends AnyFlatSpec with Matchers {
  
  "HelpTextManager" should "include command name in 'create gcc-toolchain' help text" in {
    val helpText = HelpTextManager.getSubcommandHelp("create", "gcc-toolchain")
    
    // Verify that the help text contains essential sections
    helpText should include("Command: create gcc-toolchain")
    helpText should include("DESCRIPTION:")
    helpText should include("USAGE:")
    helpText should include("ARGUMENTS:")
    helpText should include("<triple>")
    helpText should include("<destination>")
    helpText should include("OPTIONS:")
    helpText should include("--gcc-version")
    helpText should include("--binutils-version")
    helpText should include("EXAMPLES:")
  }
  
  it should "include all subcommands in 'create' help text" in {
    val helpText = HelpTextManager.getCommandHelp("create")
    
    helpText should include("Command: create")
    helpText should include("DESCRIPTION:")
    helpText should include("USAGE:")
    helpText should include("SUBCOMMANDS:")
    helpText should include("project")
    helpText should include("default-templates")
    helpText should include("gcc-toolchain")
  }
  
  it should "include project arguments in 'create project' help text" in {
    val helpText = HelpTextManager.getSubcommandHelp("create", "project")
    
    helpText should include("Command: create project")
    helpText should include("DESCRIPTION:")
    helpText should include("USAGE:")
    helpText should include("ARGUMENTS:")
    helpText should include("<template-name>")
    helpText should include("<project-name>")
  }
  
  it should "list available subcommands in invalid subcommand error message" in {
    val helpText = HelpTextManager.getInvalidSubcommandHelp("create", "invalid")
    
    helpText should include("Error: Unknown subcommand 'invalid' for command 'create'")
    helpText should include("Available subcommands for 'create':")
    helpText should include("project")
    helpText should include("default-templates")
    helpText should include("gcc-toolchain")
  }
}
