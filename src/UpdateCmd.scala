//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import scala.io.Source
import org.virtuslab.yaml.*
import sys.process._
import java.net.URL
import java.io.File
import java.nio.file.NoSuchFileException
import scala.collection.mutable

private case class TemplateHeader(name: String, root: String) derives YamlDecoder
private case class RootHeader(targets: List[String]) derives YamlDecoder

private case class Library(
    name: String,
    languages: List[String],
    dependencies: List[String]
) derives YamlDecoder

private case class Program(
    name: String,
    languages: List[String],
    dependencies: List[String]
) derives YamlDecoder

private case class Target(
    board: String,
    triple: String,
    libs: List[Library],
    programs: List[Program]
) derives YamlDecoder

private def getZigTargets(paths: Paths): String = {
  val zigTargetsReturn = os
    .proc(
      "./zig",
      "targets"
    )
    .call(
      cwd = paths.binPath
    )
  assert(zigTargetsReturn.exitCode == 0)
  zigTargetsReturn.out.string()
}

private def readTarget(target: String, paths: Paths): Option[Target] = {
  val targetTxt = os.read(paths.targetPath / (target + ".yaml"))
  val targetHeader = targetTxt.as[Target] match
    case Left(value) =>
      println(s"$value in $target template error")
      return None
    case Right(header) => header

  Some(targetHeader)
}

def updateCmd(config: Config): Unit =
  val targetPath = os.pwd / os.RelPath(config.workspace.get.getPath())
  println(s"updating workspace $targetPath")

  if !os.exists(targetPath) then
    println(s"Workspace $targetPath not found, Exiting")
    System.exit(1)

  val paths = Paths(targetPath, targetPath / "tmp", targetPath / "bin")

  if (os.exists(targetPath / "ikuy_std_resources"))
    gitUpdateLibrary(paths, "ikuy_std_resources", "main")

  // read the root.yaml
  val rootTxt = os.read(paths.targetPath / "root.yaml")
  val rootHeader = rootTxt.as[RootHeader] match
    case Left(value) =>
      println(s"$value in root.yaml template error")
      return
    case Right(header) => header

  val targets = rootHeader.targets.flatMap(readTarget(_, paths))
  // if Gcc is required (non supported LLVM target usually)
  val requiresGccCpp = targets.flatMap(target =>
    if target.libs.forall(_.languages.contains("gcc-cpp")) ||
      target.programs.forall(_.languages.contains("gcc-cpp"))
    then Some(target.triple)
    else None
  )
  // we use zig as our LLVM C++ compiler
  val requiresZig = targets
    .flatMap(target =>
      if target.libs.forall(_.languages.contains("zig")) ||
        target.libs.forall(_.languages.contains("cpp")) ||
        target.programs.forall(_.languages.contains("zig")) ||
        target.programs.forall(_.languages.contains("cpp"))
      then Some(target.triple)
      else None
    )

  // get zig targets data
  val zigTargets = getZigTargets(paths)
  val zigTargetsJson = zigTargets.as[Map[String, Any]] match
    case Left(value)  => println(s"Error $value in zig targets"); return
    case Right(value) => value

  // extract host info
  val zigHost = zigTargetsJson("native").asInstanceOf[Map[String, String]]
  val zigHostTriple = {
    // remove versions from the triple provided by zig
    zigHost("triple").split("-").map(_.split('.').head).mkString(sep = "-")
  }

  val definitions = Catalog(paths.targetPath / "ikuy_std_resources" / "definitions")
  val prefabs = Catalog(paths.targetPath / "ikuy_std_resources" / "prefabs")

  val dictionary = Map(
    "${host_triple}" -> zigHostTriple,
    "${target_path}" -> paths.targetPath.toString,
    "${bin_path}" -> paths.binPath.toString,
    "${tmp_path}" -> paths.tempPath.toString
  )

  if requiresGccCpp.nonEmpty then
    val baseTemplatePath = paths.targetPath / "ikuy_std_resources" / "templates"
    val builder = mutable.StringBuilder()
    builder ++= os.read(baseTemplatePath / "c++" / "make_compilers.sh")
    val gccFlags = "" // TODO
    builder ++= {
      for triple <- requiresGccCpp
      yield s"""
        |build_binutils $triple $${bin_path}
        |build_gcc $triple $${bin_path} "$gccFlags"
        |""".stripMargin
    }.mkString
    val builderResult = builder.result().overlordStringInterpolate(dictionary)
    os.write.over(paths.binPath / "make_compilers.sh", builderResult, perms = os.PermSet.fromString("rwxr-xr-x"))
