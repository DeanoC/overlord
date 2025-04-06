package overlord.Instances

import gagameos.{Utils, Variant}
import overlord.Hardware.{HardwareDefinition, Port}
import overlord.{
  DefinitionCatalog,
  DefinitionTrait,
  DefinitionType,
  Project,
  QueryInterface
}
import overlord.Software.SoftwareDefinition

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
  ): Option[InstanceTrait] = {

    val table = Utils.toTable(parsed)

    if (!table.contains("type")) {
      println(s"$parsed doesn't have a type")
      return None
    }

    val defTypeString = Utils.toString(table("type"))
    val name = Utils.lookupString(table, "name", defTypeString)

    val attribs: Map[String, Variant] = defaults ++ table.filter(_._1 match {
      case "type" | "name" => false
      case _               => true
    })

    val defType = DefinitionType(defTypeString)

    val defi = catalogs.findDefinition(defType) match {
      case Some(d) => d
      case None =>
        definitionFrom(catalogs, Project.projectPath, table, defType) match {
          case Some(value) => value
          case None =>
            println(s"No definition found or could be create $name $defType")
            return None
        }
    }

    defi.createInstance(name, attribs) match {
      case Some(i) => Some(i)
      case None    => None
    }
  }

  private def definitionFrom(
      catalogs: DefinitionCatalog,
      path: Path,
      table: Map[String, Variant],
      defType: DefinitionType
  ): Option[DefinitionTrait] = {

    if (table.contains("gateware")) {
      val result = HardwareDefinition(table, path)
      result match {
        case Some(value) =>
          catalogs.catalogs += (defType -> value)
          result
        case None =>
          println(s"$defType gateware was invalid")
          None
      }
    } else if (table.contains("software")) {
      val result = SoftwareDefinition(table, path)
      result match {
        case Some(value) =>
          catalogs.catalogs += (defType -> value)
          result
        case None =>
          println(s"$defType software was invalid")
          None
      }
    } else {
      println(s"$defType not found in any catalogs")
      None
    }
  }
}
