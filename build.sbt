lazy val spinalCore = ProjectRef(uri("git://github.com/deanoc/SpinalHDL.git#dev"), "core")
lazy val spinalLib = ProjectRef(uri("git://github.com/deanoc/SpinalHDL.git#dev"), "lib")
lazy val spinalPlugin = ProjectRef(uri("git://github.com/deanoc/SpinalHDL.git#dev"), "idslplugin")

val sharedSettings = Seq(
  scalaVersion        := "2.11.12",
  name                := "overlord",
  organization        := "com.deanoc",
  version             := "1.0",
  scalacOptions       ++= Seq("-deprecation",
                            (artifactPath in(spinalPlugin, Compile, packageBin)).map { file =>
                            s"-Xplugin:${file.getAbsolutePath}" }.value),
  licenses := Seq("The MIT License (MIT)" -> url("http://opensource.org/licenses/MIT")),
  libraryDependencies ++= Seq(
    "tech.sparse" %%%  "toml-scala" % "0.2.2",
    "org.scala-lang.modules" %%% "scala-parser-combinators" % "1.1.2" )
)

lazy val root = project.in(file("."))
  .aggregate(overlord.jvm, overlord.native)
  .settings(sharedSettings: _*)
  .settings(skip in publish := true)
  .dependsOn(spinalCore)
  .dependsOn(spinalLib)
  .dependsOn(spinalPlugin)

lazy val overlord =
  crossProject(JVMPlatform, NativePlatform)
    .in(file("."))
    .settings(sharedSettings)
