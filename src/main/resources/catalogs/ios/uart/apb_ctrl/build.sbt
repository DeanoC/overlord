val spinalVersion = "1.4.0"
lazy val ikuy_utils = ProjectRef(uri(
  "http://github.com/DeanoC/ikuy_utils.git#master"), "ikuy_utils")

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
  "com.github.spinalhdl" % "spinalhdl-core_2.11" % spinalVersion,
  "com.github.spinalhdl" % "spinalhdl-lib_2.11" % spinalVersion,
  "org.scala-lang.modules" %% "scala-collection-compat" % "2.3.1",
  compilerPlugin(
    "com.github.spinalhdl" % "spinalhdl-idsl-plugin_2.11" % spinalVersion
    ),
  )
lazy val apb_ctrl = (project in file("."))
  .dependsOn(ikuy_utils)

fork := true