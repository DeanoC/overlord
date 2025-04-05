val scala2Version = "2.13.13"
val scala3Version = "3.3.1" // Latest stable Scala 3 version

ThisBuild / organization := "com.deanoc"
ThisBuild / version := "1.0"
ThisBuild / licenses := Seq(
	"The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")
)

// Common settings for both Scala versions
lazy val commonSettings = Seq(
	scalacOptions += "-deprecation"
)

// Scala 2.13 dependencies
lazy val scala2Dependencies = Seq(
	"org.scala-lang.modules" %% "scala-collection-compat" % "2.3.1",
	"tech.sparse" %% "toml-scala" % "0.2.2",
	"org.scalactic" %% "scalactic" % "3.2.2",
	"org.scalatest" %% "scalatest" % "3.2.2" % "test",
	"org.scala-lang.modules" %% "scala-xml" % "1.2.0",
	"org.scala-graph" %% "graph-core" % "1.13.0",
	"org.scala-graph" %% "graph-dot" % "1.13.0",
	"org.scala-graph" %% "graph-json" % "1.13.0",
	"ca.mrvisser" %% "sealerate" % "0.0.6"
)

// Scala 3 compatible dependencies
lazy val scala3Dependencies = Seq(
	// Collection compatibility
	"org.scala-lang.modules" %% "scala-collection-compat" % "2.11.0",
	
	// Test libraries
	"org.scalactic" %% "scalactic" % "3.2.17",
	"org.scalatest" %% "scalatest" % "3.2.17" % "test",
	
	// XML handling
	"org.scala-lang.modules" %% "scala-xml" % "2.2.0",
	
	// Replacement for scala-graph using JGraphT
	"org.jgrapht" % "jgrapht-core" % "1.5.2",
	"org.jgrapht" % "jgrapht-io" % "1.5.2",
	
	// TOML parsing - using Java libraries
	"com.electronwill.night-config" % "toml" % "3.6.7",
	"com.electronwill.night-config" % "core" % "3.6.7",
	
	// Corrected scalax library for Scala 3
	"org.scala-graph" %% "graph-core" % "2.0.3",
	"org.scala-graph" %% "graph-dot" % "2.0.0"
)

// Scala 2.13 project (original)
//lazy val overlord = (project in file("."))
//	.settings(
//		name := "overlord",
//		scalaVersion := scala2Version,
//		commonSettings,
//		libraryDependencies := scala2Dependencies
//	)

lazy val overlordCommon = (project in file("."))
	.settings(
		name := "overlordCommon",
		scalaVersion := scala3Version,
		commonSettings,
		libraryDependencies := scala3Dependencies,
		maxErrors := 1,
		scalacOptions ++= Seq(
			"-feature",
			"-explain"  // Detailed error explanations
		)
	)

// Scala 3 project
lazy val overlord3 = (project in file("scala3-overlord"))
	.dependsOn(overlordCommon)
	.settings(
		name := "overlord3",
		scalaVersion := scala3Version,
		commonSettings,
		libraryDependencies := scala3Dependencies,
		maxErrors := 1,
		scalacOptions ++= Seq(
			"-feature",
			"-explain"  // Detailed error explanations
		)
	)

