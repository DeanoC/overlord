package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.hardware.{HardwareBoundrary}
import com.deanoc.overlord.definitions.HardwareDefinition
import com.deanoc.overlord.{
  DefinitionCatalog,
  Overlord,
  QueryInterface
}
import com.deanoc.overlord.definitions.{
  DefinitionTrait,
  DefinitionType
}
import com.deanoc.overlord.definitions.SoftwareDefinition
import com.deanoc.overlord.utils.{
  ArrayV,
  BigIntV,
  BooleanV,
  DoubleV,
  IntV,
  StringV,
  TableV
}
import com.deanoc.overlord.config.InstanceConfig

import java.nio.file.Path
import scala.collection.mutable
trait InstanceTrait extends QueryInterface {
  lazy val attributes: mutable.HashMap[String, Variant] =
    mutable.HashMap[String, Variant](definition.config.attributesAsVariant.toSeq: _*)
  val finalParameterTable: mutable.HashMap[String, Variant] = mutable.HashMap()

  val name: String
  val sourcePath: Path = Overlord.instancePath.toAbsolutePath

  def definition: DefinitionTrait

  def mergeAllAttributes(attribs: Map[String, Variant]): Unit =
    attribs.foreach(a => mergeAttribute(attribs, a._1))

  def mergeAttribute(attribs: Map[String, Variant], key: String): Unit = {
    if (attribs.contains(key)) {
      // overwrite existing or add it
      attributes.updateWith(key) {
        case Some(_) => Some(attribs(key))
        case None    => Some(attribs(key))
      }
    }
  }

  protected def getPort(lastName: String): Option[HardwareBoundrary] = None

  protected def wildCardMatch(
      nameId: Array[String],
      instanceId: Array[String]
  ): (Option[String], Option[HardwareBoundrary]) = {
    // wildcard match
    val is =
      for ((id, i) <- instanceId.zipWithIndex)
        yield
          if ((i < nameId.length) && (id == "_" || id == "*")) nameId(i) else id
    val ms =
      for ((id, i) <- nameId.zipWithIndex)
        yield if ((i < is.length) && (id == "_" || id == "*")) is(i) else id

    if (ms sameElements is) (Some(ms.mkString(".")), getPort(ms(0)))
    else {
      val msv = ms.reverse
      val mp = msv.head
      val tms = if (msv.length > 1) msv.tail.reverse else msv

      val port = getPort(mp)
      if ((is sameElements tms) && port.nonEmpty)
        (Some(s"${ms.mkString(".")}"), port)
      else (None, None)
    }
  }

  def getMatchNameAndPort(
      nameToMatch: String
  ): (Option[String], Option[HardwareBoundrary]) = {
    val nameWithoutBits = nameToMatch.split('[').head

    if (nameToMatch == name)
      (Some(nameToMatch), getPort(nameToMatch.split('.').last))
    else if (nameWithoutBits == name)
      (Some(nameWithoutBits), getPort(nameWithoutBits.split('.').last))
    else {
      val match0 = wildCardMatch(nameWithoutBits.split('.'), name.split('.'))
      if (match0._1.isDefined) return match0
      wildCardMatch(
        nameWithoutBits.split('.'),
        definition.defType.ident.toArray
      )
    }
  }
}

object Instance {
  def apply(
      config: InstanceConfig, // Change parameter type
      defaults: Map[String, Variant],
      catalogs: DefinitionCatalog
  ): Either[String, InstanceTrait] = {
    try {
      // Extract directly from config
      val defTypeString = config.`type`
      val name = config.name // Use name from config

      // Adjust attribs creation to use config.config
      val attribs: Map[String, Variant] =
        defaults ++ config.attributes.map { case (k, v) =>
          k -> Utils.toVariant(v) // Convert Any to Variant
        }

      val defType = DefinitionType(defTypeString)

      // TODO: one shot definitions
      val definitionResult = catalogs.findDefinition(defType) match {
        case Some(d) => Right(d)
        case None    => Left(s"No definition found or could be created for $name $defType (TODO: one shot definitions)")
      }

      definitionResult.flatMap { definition =>
        // Convert Map[String, Variant] to Option[Map[String, Any]]
        // by extracting the underlying values from Variant objects
        val attribsAsAny = attribs.map { case (k, v) =>
          k -> variantToAny(v)
        }

        definition.createInstance(name, attribsAsAny) match {
          case Right(i: InstanceTrait) => Right(i)
          case Left(error) =>
            Left(
              s"Failed to create instance for $name with definition $defType: $error"
            )
        }
      }
    } catch {
      case e: MatchError =>
        // This catch might need adjustment as the input format changes
        Left(s"Invalid instance format: ${e.getMessage()}")
      case e: Exception =>
        Left(s"Error creating instance: ${e.getMessage()}")
    }
  }

  // Helper method to convert Variant to Any
  def variantToAny(variant: Variant): Any = variant match {
    case StringV(value)  => value
    case IntV(value)     => value
    case BooleanV(value) => value
    case BigIntV(value)  => value
    case DoubleV(value)  => value
    case ArrayV(value)   => value.map(variantToAny).toSeq
    case TableV(value)   => value.map { case (k, v) => k -> variantToAny(v) }
    case null => throw new IllegalArgumentException("Variant is null")
  }
}
