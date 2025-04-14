# Template Structure Example

This document provides an example of how templates would be structured in the new CLI system.

## Template Directory Structure

Templates are stored in the `~/.overlord/templates` directory. Each template is a subdirectory containing all the files needed for a specific project type.

### Example: Bare-Metal Template

```
~/.overlord/templates/bare-metal/
├── build.sbt                  # SBT build file with appropriate dependencies
├── project/
│   ├── build.properties       # SBT version
│   └── plugins.sbt            # SBT plugins
├── README.md                  # Template documentation
├── src/
│   ├── main/
│   │   ├── resources/
│   │   │   └── logback.xml    # Logging configuration
│   │   └── scala/
│   │       └── com/
│   │           └── {{project-name}}/
│   │               ├── Main.scala             # Entry point
│   │               └── hardware/
│   │                   └── BoardConfig.scala  # Hardware configuration
│   └── test/
│       └── scala/
│           └── com/
│               └── {{project-name}}/
│                   └── MainSpec.scala         # Test file
└── {{project-name}}.over      # Overlord project file
```

### Example: Linux-App Template

```
~/.overlord/templates/linux-app/
├── build.sbt                  # SBT build file with appropriate dependencies
├── project/
│   ├── build.properties       # SBT version
│   └── plugins.sbt            # SBT plugins
├── README.md                  # Template documentation
├── src/
│   ├── main/
│   │   ├── resources/
│   │   │   └── logback.xml    # Logging configuration
│   │   └── scala/
│   │       └── com/
│   │           └── {{project-name}}/
│   │               ├── Main.scala             # Entry point
│   │               ├── app/
│   │               │   └── Application.scala  # Application logic
│   │               └── hardware/
│   │                   └── LinuxConfig.scala  # Hardware configuration
│   └── test/
│       └── scala/
│           └── com/
│               └── {{project-name}}/
│                   └── MainSpec.scala         # Test file
└── {{project-name}}.over      # Overlord project file
```

## Template Placeholders

Templates use placeholders that are replaced with actual values when a project is created:

- `{{project_name}}` - The project name (e.g., "MyProject")
- `{{PROJECT_NAME}}` - The project name in uppercase (e.g., "MYPROJECT")
- `{{project-name}}` - The project name in lowercase with hyphens (e.g., "my-project")

## Example Template Files

### build.sbt

```scala
val scala3Version = "3.3.5"

ThisBuild / organization := "com.{{project-name}}"
ThisBuild / version := "0.1.0"
ThisBuild / licenses := Seq(
  "The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")
)

// Enable plugins
enablePlugins(JavaAppPackaging)
enablePlugins(UniversalPlugin)
enablePlugins(DebianPlugin)

// Application packaging settings
Compile / mainClass := Some("com.{{project-name}}.Main")
maintainer := "user@example.com"
packageSummary := "{{project_name}}"
packageDescription := "A {{project_name}} application created with Overlord."

// Common settings
lazy val commonSettings = Seq(
  scalacOptions += "-deprecation",
  scalacOptions += "-unchecked"
)

// Dependencies
lazy val dependencies = Seq(
  // Add your dependencies here
  "org.scala-lang.modules" %% "scala-collection-compat" % "2.11.0",
  "com.github.scopt" %% "scopt" % "4.1.0",
  "com.typesafe.scala-logging" %% "scala-logging" % "3.9.5",
  "ch.qos.logback" % "logback-classic" % "1.4.11"
)

// Main project
lazy val root = (project in file("."))
  .settings(
    name := "{{project_name}}",
    scalaVersion := scala3Version,
    commonSettings,
    libraryDependencies := dependencies
  )
```

### Main.scala

```scala
package com.{{project-name}}

import com.typesafe.scalalogging.LazyLogging

object Main extends LazyLogging {
  def main(args: Array[String]): Unit = {
    logger.info("Starting {{project_name}} application")

    // Your application code here

    logger.info("{{project_name}} application completed")
  }
}
```

### {{project-name}}.over

```yaml
# {{project_name}} Overlord Project File
name: {{project_name}}
version: 1.0.0

# Hardware configuration
board: tangnano9k  # Default board, can be overridden with --board

# Project structure
structure:
  - type: chip
    name: main_processor
    definition: tangnano9k_cpu

  - type: ram
    name: main_memory
    definition: tangnano9k_ram

  - type: io
    name: uart
    definition: tangnano9k_uart

# Connections
connections:
  - connect: main_processor.bus -> main_memory.bus
  - connect: main_processor.uart -> uart.uart
```

## Using Templates

To create a new project from a template:

```bash
overlord create from-template bare-metal my-project
```

This will:
1. Create a new directory called `my-project` in the current working directory
2. Copy all files from the `bare-metal` template
3. Replace all placeholders with the project name
4. Initialize the project structure

The result is a fully functional project that can be built and run immediately.

## Adding New Templates

Users can create their own templates by:

1. Creating a new directory in `~/.overlord/templates/`
2. Adding all necessary files with appropriate placeholders
3. Creating a README.md file explaining the template

The new template will automatically be available for use with the `create from-template` command.

## Template Management

In the future, we could add commands for managing templates:

```bash
# List available templates
overlord template list

# Add a new template from a directory
overlord template add <name> <path>

# Remove a template
overlord template remove <name>

# Update a template
overlord template update <name> <path>
```

These commands would make it easier to manage templates and share them between team members.