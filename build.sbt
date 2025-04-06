val scala3Version = "3.3.1" // Latest stable Scala 3 version

ThisBuild / organization := "com.deanoc"
ThisBuild / version := "1.0"
ThisBuild / licenses := Seq(
	"The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")
)

// Enable plugins
enablePlugins(JavaAppPackaging)
enablePlugins(UniversalPlugin)
// Enable Linux plugin
enablePlugins(DebianPlugin)

// Application packaging settings
Compile / mainClass := Some("Main")
maintainer := "deano@github.com"
packageSummary := "Overlord Tool"
packageDescription := "A tool for creating complex FPGA and MCU projects with YAML-based definitions."

// Linux packaging settings
Linux / name := "overlord"
Debian / packageArchitecture := "all"  // Changed from Linux/packageArchitecture to Debian/packageArchitecture
Linux / packageName := "overlord"

// Native packager settings
executableScriptName := "overlord"

// Common settings
lazy val commonSettings = Seq(
	scalacOptions += "-deprecation",
	scalacOptions += "-unchecked"
)

// Dependencies
lazy val dependencies = Seq(
	// Collection compatibility
	"org.scala-lang.modules" %% "scala-collection-compat" % "2.11.0",
	
	// Test libraries
	"org.scalactic" %% "scalactic" % "3.2.17",
	"org.scalatest" %% "scalatest" % "3.2.17" % "test",
	
	// XML handling
	"org.scala-lang.modules" %% "scala-xml" % "2.2.0",
	
	// Graph handling
	"org.jgrapht" % "jgrapht-core" % "1.5.2",
	"org.jgrapht" % "jgrapht-io" % "1.5.2",
	"org.scala-graph" %% "graph-core" % "2.0.3",
	"org.scala-graph" %% "graph-dot" % "2.0.0",
	
	// Add SnakeYAML dependency for YAML parsing
	"org.yaml" % "snakeyaml" % "2.0"
)

// Main project (Scala 3)
lazy val root = (project in file("."))
	.settings(
		name := "overlord",
		scalaVersion := scala3Version,
		commonSettings,
		libraryDependencies := dependencies,
		maxErrors := 1,
		scalacOptions ++= Seq(
			"-feature",
			"-explain"  // Detailed error explanations
		)
	)

// Configure sbt to recognize the test directory
Test / testOptions += Tests.Argument(TestFrameworks.ScalaTest, "-oD")

import sbtassembly.AssemblyPlugin.autoImport._

assemblyMergeStrategy := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _                             => MergeStrategy.first
}

