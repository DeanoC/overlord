lazy val ikuy_utils = RootProject(uri("https://github.com/DeanoC/ikuy_utils.git#master"))

ThisBuild / version := "1.0"
ThisBuild / scalaVersion := "2.13.3"
ThisBuild / organization := "com.deanoc"
lazy val build = (project in file("."))
	.settings(
		name := "ps8",
		libraryDependencies ++= Seq(
			spinalCore,
			spinalLib,
			spinalIdslPlugin,
			"tech.sparse" %% "toml-scala" % "0.2.2",
			"org.scala-lang.modules" %% "scala-collection-compat" % "2.3.1",
			)
		)
	.dependsOn(ikuy_utils)
val spinalVersion    = "1.7.0a"
val spinalCore       = "com.github.spinalhdl" %% "spinalhdl-core" % spinalVersion
val spinalLib        = "com.github.spinalhdl" %% "spinalhdl-lib" % spinalVersion
val spinalIdslPlugin = compilerPlugin("com.github.spinalhdl" %% "spinalhdl-idsl-plugin" % spinalVersion)

fork := true