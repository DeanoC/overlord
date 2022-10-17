//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord

import scala.io.Source
import org.virtuslab.yaml.*
import sys.process._
import java.net.URL
import java.io.File
import java.nio.file.NoSuchFileException
import scala.collection.mutable

private case class Root(
    boards: List[String],
    cpus: List[String],
    software: Option[List[String]],
    gateware: Option[List[String]],
    toplevel: Option[String]
) derives YamlDecoder

private case class Software(
    name: String,
    boards: List[String],
    cpus: List[String],
    source: List[String],
    dependencies: List[String]
) derives YamlDecoder
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

private def readSoftware(target: String, paths: Paths): Option[Software] = {
  val targetTxt = os.read(paths.targetPath / (target + ".yaml"))
  val targetHeader = targetTxt.as[Software] match
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
  val rootHeader = rootTxt.as[Root] match
    case Left(value) =>
      println(s"$value in root.yaml template error")
      return
    case Right(header) => header

  // TODO generilise this to other catalog and prefix
  val definitions = Catalog(paths.targetPath / "ikuy_std_resources" / "definitions")
  val prefabs = Catalog(paths.targetPath / "ikuy_std_resources" / "prefabs")

  // use zig to get info on the host and check this version of LLVM supports all the triple we want
  // get zig targets data
  val zigTargets = {
    getZigTargets(paths) match
      case None         => return
      case Some(result) => result
  }

  rootHeader.software match
    case None =>
    case Some(softwares) =>
      val software = softwares.flatMap(sw =>
        sw.split('.').head match
          case "program" => readSoftware(sw.split('.').last, paths)
          case "lib"     => readSoftware(sw.split('.').last, paths)
          case _         => println(s"ERROR: unknown software type ${sw.split('.').head}"); None
      )
      // if Gcc is required (non supported LLVM target usually)
      val requiresGccCpp =
        software.flatMap(p => if p.source.forall(_.contains("gcc-cpp")) then Some((p.boards, p.cpus)) else None)

      // we use zig as our LLVM C++ compiler
      val requiresLlvm = software.flatMap(p =>
        if p.source.forall(t => t.contains("zig") || t.contains("cpp")) then Some((p.boards, p.cpus)) else None
      )

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
      val boardNames = (requiresGccCpp.flatMap(t => t._1) ++ requiresLlvm.flatMap(t => t._1)).toSet
      if !boardNames.forall(b =>
          definitions.matchIdentifier(Identifier(Seq("boards", b))) match
            case None        => println(s"$b board not found in catalogs"); false
            case Some(value) => true
        )
      then return

      // lets check all the cpus are available.
      val cpuNames = (requiresGccCpp.flatMap(t => t._2) ++ requiresLlvm.flatMap(t => t._2)).toSet
      if !cpuNames.forall(c =>
          definitions.matchIdentifier(Identifier(Seq("cpus", "cpu", c))) match
            case None        => println(s"$c cpu not found in catalogs"); false
            case Some(value) => true
        )
      then return

      // fetch the board files we need
      val boardsTxt = boardNames.flatMap(name =>
        definitions.fetch(Identifier(Seq("boards", name))) match
          case None       => println(s"Unknown board $name"); None
          case Some(item) => Some(os.read(item.filePath).overlordStringInterpolate(dictionary))
      )
      val boards = boardsTxt.flatMap(_.as[Board] match
        case Left(err)    => println(err); None
        case Right(value) => Some(value)
      )

      // fetch the cpus files we need
      val cpusTxt = cpuNames.flatMap(name =>
        definitions.fetch(Identifier(Seq("cpus", "cpu", name))) match
          case None       => println(s"Unknown Cpu $name"); None
          case Some(item) => Some(os.read(item.filePath).overlordStringInterpolate(dictionary))
      )
      val cpus = cpusTxt.flatMap(_.as[Cpu] match
        case Left(err)    => println(err); None
        case Right(value) => Some(value)
      )

      // lets get llvm cpu triples
      val llvmCpuNames = requiresLlvm.flatMap(t => t._2).toSet
      // get all the os and abi used
      val llvmTriples = boards
        .flatMap(b =>
          for
            cpu <- b.cpu_clusters
            llvmCpu <- llvmCpuNames
            if llvmCpu.replace("host", zigTargets.native.cpu.arch) == cpu.cpu
          yield Triple(cpu.cpu, b.os, b.abi)
        )
        .toSet

      // lets get gcc cpu triples
      val gccCpuNames = requiresGccCpp.flatMap(t => t._2).toSet
      val gccTriples = boards
        .flatMap(b =>
          for
            cpu <- b.cpu_clusters
            gccCpu <- gccCpuNames
            if gccCpu.replace("host", zigTargets.native.cpu.arch) == cpu.cpu
          yield Triple(cpu.cpu, b.os, b.abi)
        )
        .toSet

      if requiresGccCpp.nonEmpty then
        val baseTemplatePath = paths.targetPath / "ikuy_std_resources" / "templates"
        val builder = mutable.StringBuilder()
        builder ++= os.read(baseTemplatePath / "c++" / "make_compilers.sh")
        val gccFlags = "" // TODO
        builder ++= {
          for triple <- gccTriples
          yield s"""
              |build_binutils ${triple.toString} $${bin_path}
              |build_gcc ${triple.toString} $${bin_path} "$gccFlags"
              |""".stripMargin
        }.mkString
        val builderResult = builder.result().overlordStringInterpolate(dictionary)
        os.write.over(paths.binPath / "make_compilers.sh", builderResult, perms = os.PermSet.fromString("rwxr-xr-x"))
