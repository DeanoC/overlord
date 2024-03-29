ThisBuild / scalaVersion := "2.13.11"
ThisBuild / organization := "com.deanoc"
ThisBuild / version := "1.0"
ThisBuild / scalacOptions += "-deprecation"
ThisBuild / licenses := Seq(
	"The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")
	)

ThisBuild / libraryDependencies := Seq(
	"org.scala-lang.modules" %% "scala-collection-compat" % "2.3.1",
	"tech.sparse" %% "toml-scala" % "0.2.2",
	"org.scalactic" %% "scalactic" % "3.2.2",
	"org.scalatest" %% "scalatest" % "3.2.2" % "test",
	"org.scala-lang.modules" %% "scala-xml" % "1.2.0",
	"org.scala-graph" %% "graph-core" % "1.13.0",
	"org.scala-graph" %% "graph-dot" % "1.13.0",
	"org.scala-graph" %% "graph-json" % "1.13.0",
	"ca.mrvisser" %% "sealerate" % "0.0.6",
	)
lazy val overlord = (project in file("."))
