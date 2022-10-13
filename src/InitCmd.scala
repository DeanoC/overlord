//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import scala.io.Source
import org.virtuslab.yaml.*
import sys.process._
import java.net.URL
import java.io.File
import java.nio.file.NoSuchFileException

case class InitPaths(
    targetPath: os.Path,
    tempPath: os.Path,
    binPath: os.Path
)

def initCmd(config: Config): Unit =
  val targetPath = os.pwd / os.RelPath(config.workspace.get.getPath())
  println(s"initialising workspace $targetPath")

  if os.exists(targetPath) && !config.force then
    println(s"Workspace already exists, Exiting")
    System.exit(1)

  assert(config.workspace.isDefined)

  // create directories
  val paths = {
    os.makeDir.all(targetPath)
    assert(os.exists(targetPath))
    val tempPath = targetPath / "tmp"
    os.makeDir.all(tempPath)
    val binPath = targetPath / "bin"
    os.makeDir.all(binPath)
    InitPaths(targetPath, tempPath, binPath)
  }

  if !config.nogit then
    setupGit(paths)
    initialGitCommit(paths)

  if !config.nostdresources then
    installStdResources(paths)
    setupFromTemplate(paths, config.template)

  println(s"Overload workspace ${targetPath} initialised")

  updateCmd(config)

private def setupGit(paths: InitPaths): Unit =
  // setup git
  val initReturn = os
    .proc(
      "git",
      "init"
// need newer git      "--initial-branch=main"
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(initReturn.exitCode == 0)
  val oldSkoolInitialBranchReturn = os
    .proc(
      "git",
      "checkout",
      "-b",
      "main"
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(initReturn.exitCode == 0)

  os.write(paths.targetPath / ".gitignore", "")
  val addReturn = os
    .proc(
      "git",
      "add",
      ".gitignore"
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(addReturn.exitCode == 0)

private def installStdResources(paths: InitPaths): Unit =
  // add std resource subtree
  if !os.exists(paths.targetPath / "ikuy_std_resource") then
    val remoteResult = os
      .proc(
        "git",
        "remote",
        "add",
        "-f",
        "ikuy_std_resources",
        "git@github.com:DeanoC/ikuy_std_resources.git"
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(remoteResult.exitCode == 0)
    val mergeResult = os
      .proc(
        "git",
        "merge",
        "-s",
        "ours",
        "--no-commit",
        "--allow-unrelated-histories",
        "ikuy_std_resources/main"
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(mergeResult.exitCode == 0)

    val subtreeResult = os
      .proc(
        "git",
        "read-tree",
        "--prefix",
        "ikuy_std_resources/",
        "-u",
        "ikuy_std_resources/main"
      )
      .call(
        cwd = paths.targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(subtreeResult.exitCode == 0)
  val commitReturn = os
    .proc(
      "git",
      "commit",
      "-m",
      """"Subtree merged in ikuy_std_resources""""
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(commitReturn.exitCode == 0)

private def initialGitCommit(paths: InitPaths): Unit =
  val commitReturn = os
    .proc(
      "git",
      "commit",
      "-m",
      """"Initial Empty Commit""""
    )
    .call(
      cwd = paths.targetPath,
      check = false,
      stdout = os.Inherit,
      mergeErrIntoOut = true
    )
  assert(commitReturn.exitCode == 0)
