package com.deanoc.overlord.instances

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.hardware.{HardwareDefinition, Port}
import com.deanoc.overlord.{
  DefinitionCatalog,
  DefinitionTrait,
  DefinitionType,
  Project,
  QueryInterface
}
import com.deanoc.overlord.software.SoftwareDefinition

import java.nio.file.Path
import scala.collection.mutable

trait InstanceTrait extends QueryInterface {
  lazy val attributes: mutable.HashMap[String, Variant] =
    mutable.HashMap[String, Variant](definition.attributes.toSeq: _*)
  val finalParameterTable: mutable.HashMap[String, Variant] = mutable.HashMap()

  val name: String
  val sourcePath: Path = Project.instancePath.toAbsolutePath

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

  protected def getPort(lastName: String): Option[Port] = None

  protected def wildCardMatch(
      nameId: Array[String],
      instanceId: Array[String]
  ): (Option[String], Option[Port]) = {
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
  ): (Option[String], Option[Port]) = {
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
      parsed: Variant,
      defaults: Map[String, Variant],
      catalogs: DefinitionCatalog
  ): Either[String, InstanceTrait] = {
    try {
      if (parsed == null) {
        return Left("Instance definition is null")
      }

      val table = Utils.toTable(parsed)

      if (!table.contains("type")) {
        Left(s"$parsed doesn't have a type")
      } else {
        val defTypeString = Utils.toString(table("type"))
        val name = Utils.lookupString(table, "name", defTypeString)

        val attribs: Map[String, Variant] =
          defaults ++ table.filter(_._1 match {
            case "type" | "name" => false
            case _               => true
          })

        val defType = DefinitionType(defTypeString)

        for {
          definition <- catalogs.findDefinition(defType) match {
            case Some(d) => Right(d)
            case None =>
              definitionFrom(
                catalogs,
                Project.projectPath,
                table,
                defType
              ) match {
                case Right(value) => Right(value)
                case Left(error) =>
                  Left(
                    s"No definition found or could be created for $name $defType: $error"
                  )
              }
          }
          instance <- definition.createInstance(name, attribs) match {
            case Right(i: InstanceTrait) => Right(i)
            case Left(error) =>
              Left(
                s"Failed to create instance for $name with definition $defType: $error"
              )
          }
        } yield instance
      }
    } catch {
      case e: MatchError =>
        Left(s"Invalid instance format: ${e.getMessage()}")
      case e: Exception =>
        Left(s"Error creating instance: ${e.getMessage()}")
    }
  }

  private def definitionFrom(
      catalogs: DefinitionCatalog,
      path: Path,
      table: Map[String, Variant],
      defType: DefinitionType
  ): Either[String, DefinitionTrait] = {

    if (table.contains("gateware")) {
      HardwareDefinition(table, path) match {
        case Right(value) =>
          catalogs.catalogs += (defType -> value)
          Right(value)
        case Left(error) =>
          Left(s"$defType gateware was invalid: $error")
      }
    } else if (table.contains("software")) {
      SoftwareDefinition(table, path) match {
        case Some(value) =>
          catalogs.catalogs += (defType -> value)
          Right(value)
        case None =>
          Left(s"$defType software was invalid")
      }
    } else {
      Left(s"$defType not found in any catalogs")
    }
  }
}
