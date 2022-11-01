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

  updateCmd(config)
