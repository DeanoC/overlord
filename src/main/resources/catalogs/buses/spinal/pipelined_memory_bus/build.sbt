val spinalVersion = "1.4.0"

lazy val root = (project in file(".")).
	settings(
		inThisBuild(List(
			organization := "com.github.spinalhdl",
			scalaVersion := "2.11.12",
			version := "2.0.0"
			)),
		libraryDependencies ++= Seq(
			"tech.sparse" %% "toml-scala" % "0.2.2",
			"com.github.spinalhdl" % "spinalhdl-core_2.11" % spinalVersion,
			"com.github.spinalhdl" % "spinalhdl-lib_2.11" % spinalVersion,
			compilerPlugin(
				"com.github.spinalhdl" % "spinalhdl-idsl-plugin_2.11" % spinalVersion),
			),
		name := "pipelined_memory_bus"
		)

fork := true