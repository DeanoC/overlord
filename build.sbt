val scala3Version = "3.3.5" // Updated to the latest recommended Scala 3 version

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
Compile / mainClass := Some("com.deanoc.overlord.Main")
maintainer := "deano@github.com"
packageSummary := "Overlord Tool"
packageDescription := "A tool for creating complex FPGA and MCU projects with YAML-based definitions."

// Linux packaging settings
Linux / name := "overlord"
Debian / packageArchitecture := "all" // Changed from Linux/packageArchitecture to Debian/packageArchitecture
Linux / packageName := "overlord"

// Native packager settings
executableScriptName := "overlord"

// JVM memory configuration to address GC warnings
Universal / javaOptions ++= Seq(
  "-Xmx2G",
  "-XX:+UseG1GC",
  "-XX:MaxGCPauseMillis=200"
)

// Fork JVM for both run and test to apply these settings
fork := true
run / javaOptions ++= Seq(
  "-Xmx2G", 
  "-XX:+UseG1GC",
  "-XX:MaxGCPauseMillis=200",
  "-XX:+HeapDumpOnOutOfMemoryError"
)

Test / fork := true
Test / javaOptions ++= Seq(
  "-Xmx2G",
  "-XX:+UseG1GC",
  "-XX:MaxGCPauseMillis=200"
)

// Packaging workflow notes:
// - 'sbt stage' will compile and prepare the application in target/universal/stage/
// - 'sbt universal:packageBin' creates a zip package (includes compilation)
// - 'sbt debian:packageBin' creates a .deb package (includes compilation)
// - If you need a fat JAR, run 'sbt assembly' separately

// Common settings
lazy val commonSettings = Seq(
  scalacOptions += "-deprecation",
  scalacOptions += "-unchecked"
)

// Dependencies
lazy val dependencies = Seq(
  // Collection compatibility
  "org.scala-lang.modules" %% "scala-collection-compat" % "2.11.0",

  // Parser Combinators - needed for VerilogModuleParser
  "org.scala-lang.modules" %% "scala-parser-combinators" % "2.3.0",

  // Test libraries
  "org.scalactic" %% "scalactic" % "3.2.17",
  "org.scalatest" %% "scalatest" % "3.2.17" % "test",
  "org.scalatestplus" %% "mockito-4-11" % "3.2.17.0" % "test",
  "org.mockito" % "mockito-core" % "4.11.0" % "test",
  "org.scalatestplus" %% "scalacheck-1-17" % "3.2.17.0" % "test",

  // XML handling
  "org.scala-lang.modules" %% "scala-xml" % "2.2.0",

  // Graph handling
  "org.jgrapht" % "jgrapht-core" % "1.5.2",
  "org.jgrapht" % "jgrapht-io" % "1.5.2",
  "org.scala-graph" %% "graph-core" % "2.0.3",
  "org.scala-graph" %% "graph-dot" % "2.0.0",

  // Command line handling
  "com.github.scopt" %% "scopt" % "4.1.0",

  "com.typesafe.scala-logging" %% "scala-logging" % "3.9.5",
  "ch.qos.logback" % "logback-classic" % "1.4.11",

  // Circe and circe-yaml for type-safe YAML decoding
  "io.circe" %% "circe-core" % "0.14.6",
  "io.circe" %% "circe-generic" % "0.14.6", // Needed for automatic case class derivation
  "io.circe" %% "circe-parser" % "0.14.6", // Needed for parsing JSON/YAML strings
  "io.circe" %% "circe-yaml" % "0.14.1", // Use a compatible version
)

// Main project (Scala 3)
lazy val overlord = (project in file("."))
  .settings(
    name := "overlord",
    scalaVersion := scala3Version,
    commonSettings,
    libraryDependencies := dependencies,
    scalacOptions ++= Seq(
//      "-Yexplicit-nulls", // Enable null safety in Scala 3
      "-feature"
//      "-explain" // Detailed error explanations
    ),
  )
    // Ensure build_baremetal_toolchain.sh in resources is always up to date with scripts/
    Compile / resourceGenerators += Def.task {
      val src = baseDirectory.value / "scripts" / "build_baremetal_toolchain.sh"
      val dest = (Compile / resourceManaged).value / "build_baremetal_toolchain.sh"
      IO.copyFile(src, dest)
      Seq(dest)
    }.taskValue

// Ensure the main Scala source directory is included
Compile / unmanagedSourceDirectories += baseDirectory.value / "src" / "main" / "scala"

// Configure sbt to recognize the test directory
Test / testOptions += Tests.Argument(TestFrameworks.ScalaTest, "-oD")
// Ensure Main is not executed during tests
Test / fork := true
Test / mainClass := None

import sbtassembly.AssemblyPlugin.autoImport._

assemblyMergeStrategy := {
  case PathList("META-INF", xs @ _*) => MergeStrategy.discard
  case _                             => MergeStrategy.first
}
