//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import scala.io.Source
import org.virtuslab.yaml.*
import sys.process._
import java.net.URL
import java.io.File
import java.nio.file.NoSuchFileException

def initCmd(config: Config): Unit =
  val targetPath = os.pwd / os.RelPath(config.workspace.get.getPath())
  println(s"initialising workspace $targetPath")

  if os.exists(targetPath) && !config.force then
    println(s"Workspace already exists, Exiting")
    System.exit(1)

  assert(config.workspace.isDefined)

  // create directories
  val paths = Paths(targetPath, targetPath / "tmp", targetPath / "bin", targetPath / "libs")

  if !config.nogit then
    gitInit(paths, "main")
    gitCommit(paths, "Initial Empty Commit")

  if !config.nostdresources then
    gitAddCatalog(paths, "git@github.com:DeanoC/ikuy_std_resources.git", "ikuy_std_resources", "main")
    setupFromTemplate(paths, config.template)

  // we always need zig as we use it to work out host tripple
  installZig(paths)

  println(s"Overload workspace ${targetPath} initialised")

  updateCmd(config)

private def installZig(paths: Paths): Unit =
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
