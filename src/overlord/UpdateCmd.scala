//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import scala.io.Source
import org.virtuslab.yaml.*
import sys.process._
import java.net.URL
import java.io.File
import java.nio.file.NoSuchFileException
import scala.collection.mutable

case class Root(
    boards: List[String],
    cpus: List[String],
    software: Option[List[String]],
    gateware: Option[List[String]],
    toplevel: String
) derives YamlDecoder

sealed trait SoftwareDef {
  def name: String
  def boards: List[String]
  def cpus: List[String]
  def builder: List[String]
  def libraries: List[String]
  def dependencies: List[String]
  def actions: List[String]
}

case class ZigLibDef(
    library_import: String,
    library_package: String,
    library_link: String
) derives YamlDecoder

case class LibSoftwareDef(
    override val name: String,
    override val boards: List[String],
    override val cpus: List[String],
    override val builder: List[String],
    override val libraries: List[String],
    override val dependencies: List[String],
    override val actions: List[String],
    zig: Option[ZigLibDef]
) extends SoftwareDef()
    derives YamlDecoder

case class ProgramSoftwareDef(
    override val name: String,
    override val boards: List[String],
    override val cpus: List[String],
    override val builder: List[String],
    override val libraries: List[String],
    override val dependencies: List[String],
    override val actions: List[String]
) extends SoftwareDef()
    derives YamlDecoder

private case class Cpu(
    arch: String,
    features: List[String],
    width: Int,
    max_atomic_width: Int,
    max_bitop_type_width: Int
) derives YamlDecoder

private case class CpuCluster(
    cpu: String,
    count: Int
) derives YamlDecoder
private case class Board(
    board_type: String,
    os: String,
    abi: String,
    cpu_clusters: List[CpuCluster]
) derives YamlDecoder

private def readSoftware(target: String, paths: Paths): Option[SoftwareDef] = {
  val targetTxt = os.read(paths.targetPath / (target + ".yaml"))
  val targetHeader = targetTxt.as[LibSoftwareDef] match
    case Left(value) =>
      targetTxt.as[ProgramSoftwareDef] match
        case Left(value) =>
          println(s"$value in $target error")
          return None
        case Right(header) => header
    case Right(header) => header

  Some(targetHeader)
}
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

def updateCmd(config: Config): Unit =
  val targetPath = os.pwd / os.RelPath(config.workspace.get.getPath())
  println(s"updating workspace $targetPath")

  if !os.exists(targetPath) then
    println(s"Workspace $targetPath not found, Exiting")
    System.exit(1)

  val paths = Paths(targetPath, targetPath / "tmp", targetPath / "bin", targetPath / "libs")

  if !os.exists(paths.libPath / "ltngt.packages") then
    gitAddLibSubTree(paths, "git@github.com:DeanoC/ltngt.packages.git", "ltngt.packages", "main")

  if (os.exists(paths.libPath / "ltngt.packages"))
    gitUpdateLibrary(paths, "git@github.com:DeanoC/ltngt.packages.git", "ltngt.packages", "main")

  // we always need zig as we use it to work out host tripple
  installZig(paths)

  // read the root.yaml
  val rootTxt = os.read(paths.targetPath / "root.yaml")
  val rootHeader = rootTxt.as[Root] match
    case Left(value) =>
      println(s"$value in root.yaml template error")
      return
    case Right(header) => header

  rootHeader.software match
    case None            =>
    case Some(softwares) => processSoftware(paths, softwares, config.pushBeforeFetch, config.skipGit)

private def processSoftware(paths: Paths, softwares: Seq[String], pushBeforeFetch: Boolean, skipGit: Boolean): Unit =
  // TODO generilise this to other catalogs
  val catalog = Catalog(paths.libPath / "ltngt.packages")

  // use zig to get info on the host and check this version of LLVM supports all the triple we want
  // get zig targets data
  val zigTargets = {
    getZigTargets(paths) match
      case None         => return
      case Some(result) => result
  }

  val software = softwares.flatMap(sw =>
    sw.split('.').head match
      case "program" => readSoftware(sw.split('.').last, paths)
      case "library" => readSoftware(sw.split('.').last, paths)
      case _         => println(s"ERROR: unknown software type ${sw.split('.').head}"); None
  )

  val localPrograms = softwares.flatMap(sw =>
    if sw.split('.').head == "program" then Some(Identifier(sw.split('.').toSeq.drop(1)))
    else None
  )
  val localLibraries = softwares.flatMap(sw =>
    if sw.split('.').head == "library" then Some(Identifier(sw.split('.').toSeq.drop(1)))
    else None
  )

  val requiresCMake =
    software.flatMap(p => if p.builder.forall(_ == "cmake") then Some((p.boards, p.cpus)) else None)

  // we use zig as our LLVM build system
  val requiresZig = software.flatMap(p => if p.builder.forall(_ == "zig") then Some((p.boards, p.cpus)) else None)

  val dictionary = Map(
    "${host_triple}" -> zigTargets.native.genericTriple.toString,
    "${host_cpu}" -> zigTargets.native.cpu.arch.toString,
    "${host_os}" -> zigTargets.native.os.toString,
    "${host_abi}" -> zigTargets.native.abi.toString,
    "${host_features}" -> zigTargets.native.cpu.features.mkString(","),
    "${target_path}" -> paths.targetPath.toString,
    "${bin_path}" -> paths.binPath.toString,
    "${tmp_path}" -> paths.tempPath.toString
  )

  // lets check all the boards are available.
  val boardNames = (requiresCMake.flatMap(t => t._1) ++ requiresZig.flatMap(t => t._1)).toSet
  if !boardNames.forall(b =>
      catalog.matchIdentifier(Identifier(Seq("boards", b))) match
        case None        => println(s"$b board not found in catalogs"); false
        case Some(value) => true
    )
  then return

  // lets check all the cpus are available.
  val cpuNames = (requiresZig.flatMap(t => t._2) ++ requiresZig.flatMap(t => t._2)).toSet
  if !cpuNames.forall(c =>
      catalog.matchIdentifier(Identifier(Seq("cpus", c))) match
        case None        => println(s"$c cpu not found in catalogs"); false
        case Some(value) => true
    )
  then return

  // fetch the board files we need
  val boards =
    boardNames
      .flatMap(name =>
        catalog.fetch(Identifier(Seq("boards", name))) match
          case None       => println(s"Unknown board $name"); None
          case Some(item) => Some(os.read(item.filePath).overlordStringInterpolate(dictionary))
      )
      .flatMap(_.as[Board] match
        case Left(err)    => println(err); None
        case Right(value) => Some(value)
      )
      .toSeq

  // fetch the cpus files we need
  val cpus =
    cpuNames
      .flatMap(name =>
        catalog.fetch(Identifier(Seq("cpus", name))) match
          case None       => println(s"Unknown Cpu $name"); None
          case Some(item) => Some(os.read(item.filePath).overlordStringInterpolate(dictionary))
      )
      .flatMap(_.as[Cpu] match
        case Left(err)    => println(err); None
        case Right(value) => Some(value)
      )
      .toSeq

  // get all the os and abi used
  val llvmTriples = boards
    .flatMap(b =>
      for
        cpu <- b.cpu_clusters
        llvmCpu <- requiresZig.flatMap(t => t._2).toSet
        if llvmCpu.replace("host", zigTargets.native.cpu.arch) == cpu.cpu
      yield Triple(cpu.cpu, b.os, b.abi)
    )
    .toSet

  // lets get gcc cpu triples
  val gccTriples = boards
    .flatMap(b =>
      for
        cpu <- b.cpu_clusters
        gccCpu <- requiresCMake.flatMap(t => t._2).toSet
        if gccCpu.replace("host", zigTargets.native.cpu.arch) == cpu.cpu
      yield Triple(cpu.cpu, b.os, b.abi)
    )
    .toSet

  val sw = Software(
    paths,
    zigTargets,
    catalog,
    software,
    localPrograms,
    localLibraries,
    dictionary,
    gccTriples.toSeq,
    llvmTriples.toSeq,
    boards,
    cpus,
    pushBeforeFetch,
    skipGit
  )
