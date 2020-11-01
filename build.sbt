//lazy val ikuy = ProjectRef(uri("git@github.com:DeanoC/ikuy2.git#master"), "root")
lazy val ikuy = ProjectRef(file("../ikuy2"), "ikuy")

ThisBuild / scalaVersion := "2.11.12"
ThisBuild / organization := "com.deanoc"
ThisBuild / version := "1.0"
ThisBuild / scalacOptions += "-deprecation"
ThisBuild / licenses := Seq(
        "The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")
      )
ThisBuild / libraryDependencies := Seq(
        "tech.sparse"            %% "toml-scala"               % "0.2.2",
        "org.scala-lang.modules" %% "scala-parser-combinators" % "1.0.4",
        "org.scalactic"          %% "scalactic"                % "3.2.2",
        "org.scalatest"          %% "scalatest"                % "3.2.2" % "test"
        )

//lazy val overlord = crossProject(JVMPlatform, NativePlatform)
//                      .in(file("."))
//                      .settings(sharedSettings)

lazy val parser = (project in (file("src/parser")))

lazy val cmdline = (project in (file("src/cmdline")))
  .dependsOn(parser, ikuy)

lazy val overlord = (project in (file(".")))
  .dependsOn(cmdline)
