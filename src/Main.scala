//> using scala "3.2"
//> using lib "com.github.scopt::scopt:4.0.1"
//> using lib "com.lihaoyi::os-lib:0.8.0"
//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import scopt.OParser
import java.io.File

val ProgramName = "overlord";
val ProgramVersion = "0.0";

enum Mode:
  case None, Create, Update, Init

case class Config(
    mode: Mode = Mode.None,
    input: Option[File] = None,
    workspace: Option[File] = None,
    template: String = "host_hello_world",
    nostdresources: Boolean = false,
    force: Boolean = false
)

val builder = OParser.builder[Config]
val argParser = {
  import builder._
  OParser.sequence(
    programName(ProgramName),
    head(ProgramName, ProgramVersion),
    // globals optionals
    opt[Unit]("nostdresources")
      .text("Don't get/update std resources")
      .action((_, c) => c.copy(nostdresources = true)),
    opt[Unit]("force")
      .text("Force file operations even if it overwrites existing files")
      .action((_, c) => c.copy(force = true)),
    // modes
    cmd("create")
      .action((_, c) => c.copy(mode = Mode.Create))
      .text("create a new workspace")
      .children(
        arg[File]("input")
          .required()
          .action((in, c) => c.copy(input = Some(in)))
          .text("the folder name to read from"),
        arg[File]("output")
          .required()
          .text("the folder name to generate workspace at")
          .action((out, c) => c.copy(input = Some(out)))
      ),
    cmd("update")
      .action((_, c) => c.copy(mode = Mode.Update))
      .text("update an existing workspace")
      .children(
        arg[File]("workspace")
          .required()
          .action((in, c) => c.copy(workspace = Some(in)))
          .text("the folder name of the workspace to update")
      ),
    cmd("init")
      .action((_, c) => c.copy(mode = Mode.Init))
      .text("create a new workspace")
      .children(
        arg[File]("workspace")
          .required()
          .action((in, c) => c.copy(workspace = Some(in)))
          .text("the folder name to init a workspace in"),
        opt[String]("template")
          .action((in, c) => c.copy(template = in))
          .text("which template to use (host_hello_world is default")
      ),
    // validate the config
    checkConfig { c =>
      c.mode match {
        case Mode.None => failure("Must choose a command")
        case _         => success
      }
    }
  )
}

object Main {
  def main(args: Array[String]): Unit = {
    OParser.parse(argParser, args, Config()) match {
      case Some(config) =>
        println { s"$ProgramName $ProgramVersion" };
        // do stuff with config
        config.mode match
          case Mode.Create => Create(config)
          case Mode.Update => Update(config)
          case Mode.Init   => Init(config)
          // already caught by args checker
          case Mode.None =>
      case _ =>
        System.exit(1)
    }
  }

  def Create(config: Config) = {}

  def Update(config: Config) = {}

  def Init(config: Config) = {
    val targetPath = os.pwd / os.RelPath(config.workspace.get.getPath())
    println(s"initialising workspace $targetPath");

    if (os.exists(targetPath) && !config.force) {
      println(s"Workspace already exists, Exiting")
      System.exit(1)
    }
    assert(config.workspace.isDefined)
    os.makeDir.all(targetPath)
    assert(os.exists(targetPath))

    // setup git
    val initReturn = os
      .proc(
        "git",
        "init"
      )
      .call(
        cwd = targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(initReturn.exitCode == 0)
    val commitReturn = os
      .proc(
        "git",
        "commit",
        "--allow-empty",
        "-n",
        "-m",
        """"Initial""""
      )
      .call(
        cwd = targetPath,
        check = false,
        stdout = os.Inherit,
        mergeErrIntoOut = true
      )
    assert(commitReturn.exitCode == 0)

    // add std resource subtree
    if (!config.nostdresources) {
      val subtreeResult = os
        .proc(
          "git",
          "subtree",
          "add",
          "--prefix",
          "ikuy_std_resource",
          "git@github.com:DeanoC/ikuy_std_resources.git",
          "master",
          "--squash"
        )
        .call(
          cwd = targetPath,
          check = false,
          stdout = os.Inherit,
          mergeErrIntoOut = true
        )
      assert(subtreeResult.exitCode == 0)
    }

    println("init complete")
  }
}
