package com.deanoc.overlord.cli

import com.deanoc.overlord.Resources
import com.deanoc.overlord.utils.Logging

import scopt.OParser

/** Parser for the Overlord CLI commands. Implements a hierarchical command
  * structure similar to git.
  */
object CommandLineParser extends Logging {

  /** Creates the command line parser.
    *
    * @return
    *   The parser
    */
  def createParser(): OParser[_, Config] = {
    val builder = OParser.builder[Config]
    import builder._

    // Common options that can be used with multiple commands
    val commonOptions = Seq(
      opt[Unit]("nostdresources")
        .action((_, c) => c.copy(nostdresources = true))
        .text("don't use the standard catalog"),
      opt[Unit]("nostdprefabs")
        .action((_, c) => c.copy(nostdprefabs = true))
        .text("don't use the standard prefabs"),
      opt[String]("resources")
        .action((x, c) => c.copy(resources = Some(x)))
        .text("use the specified path as the root of resources"),
      opt[Unit]("yes")
        .abbr("y")
        .action((_, c) => c.copy(yes = true))
        .text(
          "automatically agree (e.g., download resource files without prompting)"
        ),
      opt[Unit]("noexit")
        .action((_, c) => c.copy(noexit = true))
        .text("disable automatic exit on error logs"),
      opt[String]("trace")
        .action((x, c) => c.copy(trace = Some(x)))
        .text(
          "enable trace logging for comma-separated list of modules (can use short names)"
        ),
      opt[String]("debug")
        .action((x, c) => c.copy(debug = Some(x)))
        .text(
          "enable debug logging for comma-separated list of modules (can use short names)"
        ),
      opt[String]("stdresource")
        .action((x, c) => c.copy(stdresource = Some(x)))
        .text(
          s"specify the standard resource path {default: ${Resources.stdResourcePath()}}"
        )
    )

    // CREATE command and subcommands
    val createCommand = cmd("create")
      .action((_, c) => c.copy(command = Some("create")))
      .children(
        // create project subcommand (renamed from from-template)
        cmd("project")
          .action((_, c) => c.copy(subCommand = Some("project")))
          .text("Create a new project from a template")
          .children(
            arg[String]("<template-name>")
              .required()
              .action((x, c) => c.copy(templateName = Some(x)))
              .text(
                "name of the template to use (e.g., bare-metal, linux-app)"
              ),
            arg[String]("<project-name>")
              .required()
              .action((x, c) => c.copy(projectName = Some(x)))
              .text(
                "name of the project to create (will be created in the same directory as the project YAML file)"
              )
          ),

        // create default-templates subcommand
        cmd("default-templates")
          .action((_, c) => c.copy(subCommand = Some("default-templates")))
          .text("Download standard templates without creating a project")
      )

    // GENERATE command and subcommands
    val generateCommand = cmd("generate")
      .action((_, c) => c.copy(command = Some("generate")))
      .children(
        // generate test subcommand
        cmd("test")
          .action((_, c) => c.copy(subCommand = Some("test")))
          .text("Generate test files for a project")
          .children(
            arg[String]("<project-name>")
              .required()
              .action((x, c) => c.copy(projectName = Some(x)))
              .text("name of the project to generate tests for")
          ),

        // generate report subcommand
        cmd("report")
          .action((_, c) => c.copy(subCommand = Some("report")))
          .text("Generate a report about the project structure")
          .children(
            arg[String]("<infile>")
              .required()
              .action((x, c) => c.copy(infile = Some(x)))
              .text("filename should be a .yaml file to use for the project")
          ),

        // generate svd subcommand
        cmd("svd")
          .action((_, c) => c.copy(subCommand = Some("svd")))
          .text("Generate a CMSIS-SVD file")
          .children(
            arg[String]("<infile>")
              .required()
              .action((x, c) => c.copy(infile = Some(x)))
              .text("filename should be a .yaml file to use for the project")
          )
      )

    // CLEAN command and subcommands
    val cleanCommand = cmd("clean")
      .action((_, c) => c.copy(command = Some("clean")))
      .children(
        // clean test subcommand
        cmd("test")
          .action((_, c) => c.copy(subCommand = Some("test")))
          .text("Clean test files for a project")
          .children(
            arg[String]("<project-name>")
              .required()
              .action((x, c) => c.copy(projectName = Some(x)))
              .text("name of the project to clean tests for")
          )
      )

    // UPDATE command and subcommands
    val updateCommand = cmd("update")
      .action((_, c) => c.copy(command = Some("update")))
      .children(
        // update project subcommand
        cmd("project")
          .action((_, c) => c.copy(subCommand = Some("project")))
          .text("Update an existing project")
          .children(
            arg[String]("<infile>")
              .required()
              .action((x, c) => c.copy(infile = Some(x)))
              .text("filename should be a .yaml file to use for the project"),
            opt[String]("instance")
              .action((x, c) => c.copy(instance = Some(x)))
              .text("specify the instance to update")
          ),

        // update catalog subcommand
        cmd("catalog")
          .action((_, c) => c.copy(subCommand = Some("catalog")))
          .text("Update the catalog from the remote repository")
      )

    // TEMPLATE command and subcommands
    val templateCommand = cmd("template")
      .action((_, c) => c.copy(command = Some("template")))
      .children(
        // template list subcommand
        cmd("list")
          .action((_, c) => c.copy(subCommand = Some("list")))
          .text("List all available templates"),

        // template add subcommand
        cmd("add")
          .action((_, c) => c.copy(subCommand = Some("add")))
          .text("Add a local template")
          .children(
            arg[String]("<name>")
              .required()
              .action((x, c) => c.copy(templateName = Some(x)))
              .text("name of the template"),
            arg[String]("<path>")
              .required()
              .action((x, c) => {
                c.options += ("path" -> x)
                c
              })
              .text("path to the template directory")
          ),

        // template add-git subcommand
        cmd("add-git")
          .action((_, c) => c.copy(subCommand = Some("add-git")))
          .text("Add a template from a git repository")
          .children(
            arg[String]("<name>")
              .required()
              .action((x, c) => c.copy(templateName = Some(x)))
              .text("name of the template"),
            arg[String]("<git-url>")
              .required()
              .action((x, c) => c.copy(gitUrl = Some(x)))
              .text("URL of the git repository"),
            opt[String]("branch")
              .action((x, c) => c.copy(branch = Some(x)))
              .text("branch to use (default: main)")
          ),

        // template add-github subcommand
        cmd("add-github")
          .action((_, c) => c.copy(subCommand = Some("add-github")))
          .text("Add a template from GitHub")
          .children(
            arg[String]("<name>")
              .required()
              .action((x, c) => c.copy(templateName = Some(x)))
              .text("name of the template"),
            arg[String]("<owner/repo>")
              .required()
              .action((x, c) => c.copy(ownerRepo = Some(x)))
              .text("GitHub repository in the format 'owner/repo'"),
            opt[String]("ref")
              .action((x, c) => c.copy(ref = Some(x)))
              .text(
                "reference to use (tag, branch, or commit hash, default: main)"
              )
          ),

        // template remove subcommand
        cmd("remove")
          .action((_, c) => c.copy(subCommand = Some("remove")))
          .text("Remove a template")
          .children(
            arg[String]("<name>")
              .required()
              .action((x, c) => c.copy(templateName = Some(x)))
              .text("name of the template to remove")
          ),

        // template update subcommand
        cmd("update")
          .action((_, c) => c.copy(subCommand = Some("update")))
          .text("Update a template from its source")
          .children(
            arg[String]("<name>")
              .required()
              .action((x, c) => c.copy(templateName = Some(x)))
              .text("name of the template to update")
          ),

        // template update-all subcommand
        cmd("update-all")
          .action((_, c) => c.copy(subCommand = Some("update-all")))
          .text("Update all templates from their sources")
      )

    // Combine all commands and options
    OParser.sequence(
      programName("overlord"),
      head("overlord", "1.0"),
      createCommand,
      generateCommand,
      cleanCommand,
      updateCommand,
      templateCommand,
      help("help").text("prints this usage text"),
      // Add common options at the top level
      opt[Unit]("nostdresources")
        .action((_, c) => c.copy(nostdresources = true))
        .text("don't use the standard catalog"),
      opt[Unit]("nostdprefabs")
        .action((_, c) => c.copy(nostdprefabs = true))
        .text("don't use the standard prefabs"),
      opt[String]("resources")
        .action((x, c) => c.copy(resources = Some(x)))
        .text("use the specified path as the root of resources"),
      opt[Unit]("yes")
        .abbr("y")
        .action((_, c) => c.copy(yes = true))
        .text(
          "automatically agree (e.g., download resource files without prompting)"
        ),
      opt[Unit]("noexit")
        .action((_, c) => c.copy(noexit = true))
        .text("disable automatic exit on error logs"),
      opt[String]("trace")
        .action((x, c) => c.copy(trace = Some(x)))
        .text(
          "enable trace logging for comma-separated list of modules (can use short names)"
        ),
      opt[String]("debug")
        .action((x, c) => c.copy(debug = Some(x)))
        .text(
          "enable debug logging for comma-separated list of modules (can use short names)"
        ),
      opt[String]("stdresource")
        .action((x, c) => c.copy(stdresource = Some(x)))
        .text(
          s"specify the standard resource path {default: ${Resources.stdResourcePath()}}"
        )
    )
  }

  /** Parses command line arguments.
    *
    * @param args
    *   The command line arguments
    * @return
    *   Some(config) if parsing was successful, None otherwise
    */
  def parse(args: Array[String]): Option[Config] = {
    val initialConfig = Config()
    val isNonInteractive = System.console() == null
    val parser = createParser()

    // Handle empty arguments case - show help directly
    if (args.isEmpty || args.contains("--help") || args.contains("-h")) {
      println(OParser.usage(parser)) // Print usage information
      sys.exit(0) // Exit gracefully
      return None
    }

    OParser.parse(
      parser,
      args,
      initialConfig.copy(yes = initialConfig.yes || isNonInteractive)
    )
  }
}
