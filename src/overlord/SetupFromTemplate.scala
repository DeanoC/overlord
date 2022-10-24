//> using lib "org.virtuslab::scala-yaml:0.0.5"
//> using lib "org.typelevel::cats-parse:0.3.8"
package Overlord

import org.virtuslab.yaml.*
import sys.process._
import scala.io.Source
private case class TemplateHeader(name: String, root: String) derives YamlDecoder

def setupFromTemplate(paths: Paths, templateName: String): Unit =
  val templatePath = paths.targetPath / "ikuy_std_resources" / "templates" / "init" / templateName
  if !os.exists(templatePath) then
    println(s"Unable to find $templateName in std resources")
    return

  // copy files
  os.copy(templatePath, paths.targetPath, replaceExisting = true, mergeFolders = true)

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
