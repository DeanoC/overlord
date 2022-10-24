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
  case None, Update, Init

case class Config(
    mode: Mode = Mode.None,
    input: Option[File] = None,
    workspace: Option[File] = None,
    force: Boolean = false,

    // update items
    pushBeforeFetch: Boolean = false,

    // init command config items
    template: String = "host_hello_world",
    nostdresources: Boolean = false,
    nogit: Boolean = false
)

case class Paths(
    targetPath: os.Path,
    tempPath: os.Path,
    binPath: os.Path,
    libPath: os.Path
):
  if !os.exists(targetPath) then os.makeDir.all(targetPath)
  if !os.exists(tempPath) then os.makeDir.all(tempPath)
  if !os.exists(binPath) then os.makeDir.all(binPath)
  if !os.exists(libPath) then os.makeDir.all(libPath)

val builder = OParser.builder[Config]
val argParser = {
  import builder._
  OParser.sequence(
    programName(ProgramName),
    head(ProgramName, ProgramVersion),
    opt[Unit]("force")
      .text("Force operations even if it overwrites existing files")
      .action((_, c) => c.copy(force = true)),
    arg[File]("workspace")
      .required()
      .action((in, c) => c.copy(workspace = Some(in)))
      .text("the folder name of the workspace"),
    // modes
    cmd("update")
      .action((_, c) => c.copy(mode = Mode.Update))
      .text("update an existing workspace")
      .children(
        opt[Unit]("push_before_fetch")
          .text("Try and push any commits before fetching")
          .action((_, c) => c.copy(pushBeforeFetch = true))
      ),
    cmd("init")
      .action((_, c) => c.copy(mode = Mode.Init))
      .text("create a new workspace")
      .children(
        opt[String]("template")
          .action((in, c) => c.copy(template = in))
          .text("which template to use (host_hello_world is default"),
        opt[Unit]("nostdresources")
          .text("Don't get std resources")
          .action((_, c) => c.copy(nostdresources = true)),
        opt[Unit]("nogit")
          .text("Don't setup git")
          .action((_, c) => c.copy(nogit = true))
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
  def main(args: Array[String]): Unit =
    OParser.parse(argParser, args, Config()) match {
      case Some(config) =>
        println { s"$ProgramName $ProgramVersion" };
        // do stuff with config
        config.mode match
          case Mode.Update => updateCmd(config)
          case Mode.Init   => initCmd(config)
          // already caught by args checker
          case Mode.None =>
      case _ =>
        System.exit(1)
    }

}
