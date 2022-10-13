//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import org.virtuslab.yaml.*
import sys.process._
import scala.io.Source

def setupFromTemplate(paths: InitPaths, templateName: String): Unit =
  val templatePath = paths.targetPath / "ikuy_std_resources" / "templates" / "init" / templateName
  if !os.exists(templatePath) then
    println(s"Unable to find $templateName in std resources")
    return

  // we always need zig as we use it to work out host tripple
  installZig(paths)

  // copy files
  os.copy(templatePath, paths.targetPath, replaceExisting = true, createFolders = true, mergeFolders = true)

  // read template header file
  val templateTxt = os.read(paths.targetPath / (templateName + ".yaml"))
  val templateHeader = templateTxt.as[TemplateHeader] match
    case Left(value) =>
      println(s"$value in $templateName template error")
      return
    case Right(header) => header
  println(s"Using ${templateHeader.name} template")

  // remove the template from the workspace now
  os.remove(paths.targetPath / (templateName + ".yaml"))

  println(s"Setup finished from template $templateName")

private def installZig(paths: InitPaths): Unit =
  // download zig for linux
  val zigEntry: Map[String, String] =
    Source.fromURL("https://ziglang.org/download/index.json").mkString.as[Map[String, Any]] match
      case Left(value) =>
        println(value.msg)
        println("Unable to find zig toolchain")
        return
      case Right(value) =>
        value
          .asInstanceOf[Map[String, Any]]("master")
          .asInstanceOf[Map[String, Map[String, String]]]("x86_64-linux")

  val tarballNameWithExt = zigEntry("tarball").split('/').last
  val tarballName = tarballNameWithExt.replaceAllLiterally(".tar.xz", "")

  if !os.exists(paths.tempPath / tarballNameWithExt) then
    println(s"Fetching ${zigEntry("tarball")}")
    val zigFetchResult = os
      .proc(
        "curl",
        "--url",
        zigEntry("tarball"),
        "--output",
        (paths.tempPath / tarballNameWithExt).toString()
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(zigFetchResult.exitCode == 0)

  if !os.exists(paths.tempPath / tarballName) then
    val tarResult = os
      .proc(
        "tar",
        "xvf",
        tarballNameWithExt.toString()
      )
      .call(
        cwd = paths.tempPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(tarResult.exitCode == 0)

  os.copy(paths.tempPath / tarballName, paths.binPath, replaceExisting = true, mergeFolders = true)

  println("Zig installed to bin")
