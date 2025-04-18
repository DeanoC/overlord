package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.ChipDefinitionTrait
import com.deanoc.overlord.interfaces.RamLike

import scala.reflect.ClassTag

case class RamInstance(
    name: String,
    override val definition: ChipDefinitionTrait
) extends ChipInstance
    with RamLike {
  private val cpuRegEx = "\\s*,\\s*".r

  private def decodeCpusString(cpus: String): Seq[String] =
    if (cpus == "_") Seq() else cpuRegEx.split(cpus).toSeq.map(_.toLowerCase())

  private lazy val ranges: Seq[(BigInt, BigInt, Boolean, Seq[String])] = {
    if (!attributes.contains("ranges")) Seq()
    else
      Utils.toArray(attributes("ranges")).toIndexedSeq.map { b =>
        (
          Utils.lookupBigInt(Utils.toTable(b), "address", 0),
          Utils.lookupBigInt(Utils.toTable(b), "size", 0),
          Utils.lookupBoolean(Utils.toTable(b), "fixed_address", false),
          decodeCpusString(
            Utils.lookupString(Utils.toTable(b), key = "cpus", "_")
          )
        )
      }
  }

  override def isVisibleToSoftware: Boolean = true

  override def getInterface[T](implicit tag: ClassTag[T]): Option[T] = {
    val RamLike_ = classOf[RamLike]
    tag.runtimeClass match {
      case RamLike_ => Some(asInstanceOf[T])
      case _        => super.getInterface[T](tag)
    }

  }

  override def getRanges: Seq[(BigInt, BigInt, Boolean, Seq[String])] = ranges
}

object RamInstance {
  def apply(
      ident: String,
      definition: ChipDefinitionTrait,
      attribs: Map[String, Variant]
  ): Either[String, RamInstance] = {
    if (
      (!definition.attributes
        .contains("ranges")) && (!attribs.contains("ranges"))
    ) {
      Left(s"ERROR: ram ${ident} has no ranges, so isn't a valid range")
    } else {
      val ram = RamInstance(ident, definition)
      ram.mergeAllAttributes(attribs)
      Right(ram)
    }
  }
}
