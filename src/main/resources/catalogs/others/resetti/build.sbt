lazy val ikuy_utils = ProjectRef(uri(
	"http://github.com/DeanoC/ikuy_utils.git#master"), "ikuy_utils")
val spinalVersion = "1.7.0a"

ThisBuild / scalaVersion := "2.11.12"
ThisBuild / organization := "com.deanoc"
ThisBuild / version := "1.0"
ThisBuild / scalacOptions += "-deprecation"
ThisBuild / scalacOptions += "-target:jvm-1.8"
ThisBuild / licenses := Seq(
	"The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")
	)

ThisBuild / libraryDependencies := Seq(
	"tech.sparse" %% "toml-scala" % "0.2.2",
	"org.scala-lang.modules" %% "scala-collection-compat" % "2.3.1",

	"com.github.spinalhdl" %% "spinalhdl-core" % spinalVersion,
	"com.github.spinalhdl" %% "spinalhdl-lib" % spinalVersion,
	compilerPlugin(
		"com.github.spinalhdl" %% "spinalhdl-idsl-plugin" % spinalVersion
		),
	)
lazy val resetti = (project in file("."))
  .dependsOn(ikuy_utils)

fork := true