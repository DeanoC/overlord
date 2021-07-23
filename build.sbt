//lazy val ikuy = ProjectRef(uri("git@github.com:DeanoC/ikuy2.git#master"),
// "root")
//lazy val ikuy = ProjectRef(file("../ikuy2"), "ikuy")
lazy val ikuy_utils = ProjectRef(uri(
	"http://github.com/DeanoC/ikuy_utils.git#master"), "ikuy_utils")
//lazy val ikuy_utils = ProjectRef(file("../ikuy_utils"), "ikuy_utils")


ThisBuild / scalaVersion := "2.13.3"
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
	"org.scala-lang.modules" %% "scala-xml" % "1.2.0"
	)
lazy val overlord = (project in file("."))
	.dependsOn(ikuy_utils)
