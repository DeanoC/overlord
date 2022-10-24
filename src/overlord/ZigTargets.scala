//> using lib "org.virtuslab::scala-yaml:0.0.5"
package Overlord
import org.virtuslab.yaml.*

case class Triple(
    arch: String,
    os: String,
    abi: String
):
  override def toString: String = s"$arch.$os.$abi"

object Triple:
  def apply(triple: String): Triple = {
    val split: Array[String] = triple.split('.')
    Triple(split(0), split(1), split(2))
  }

case class ZigNativeCpu(
    arch: String,
    name: String,
    features: List[String]
) derives YamlDecoder

case class ZigNative(
    triple: String,
    cpu: ZigNativeCpu,
    os: String,
    abi: String
) derives YamlDecoder:
  val genericTriple = Triple(cpu.arch, os, abi)

case class ZigTargets(
    arch: List[String],
    os: List[String],
    abi: List[String],
    libc: List[String],
    glibc: List[String],
    cpus: Map[String, Map[String, List[String]]],
    native: ZigNative
) derives YamlDecoder:
  def hasTarget(target: Triple): Boolean =
    arch.contains(target.arch) && os.contains(target.os) && abi.contains(target.abi)

def getZigTargets(paths: Paths): Option[ZigTargets] =
  val zigTargetsReturn = os
    .proc(
      "./zig",
      "targets"
    )
    .call(
      cwd = paths.binPath
    )
  assert(zigTargetsReturn.exitCode == 0)
  zigTargetsReturn.out.string().as[ZigTargets] match
    case Left(err)     => println(s"Error: ZigTarget error $err"); None
    case Right(result) => Some(result)
