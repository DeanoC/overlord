package overlord

import gagameos._
import overlord.Project

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer

class DefinitionCatalog {
  type key = DefinitionType
  type value = DefinitionTrait
  type keyStore = mutable.HashMap[key, value]

  val catalogs: keyStore = mutable.HashMap()

  def printDefinitionCatalogs(): Unit = {
    println("Definition Catalogs:")
    for { k <- catalogs.keys } {
      println(s"k  ${k.ident.mkString(".")} ${k.getClass}")
    }
  }
  def findDefinition(name: String): Option[value] = {
    val ident = name.split('.').toList
    var defi: Option[value] = None

    for {
      k <- catalogs.keys
      if k.getClass == classOf[DefinitionType]
      if k.ident.length >= ident.length
    } {
      var curMatch = 0
      for {
        (s, i) <- k.ident.zipWithIndex
        if i < ident.length
        if i == curMatch
        if s == ident(i)
      } curMatch += 1

      if (curMatch >= ident.length) {
        defi = Some(catalogs(k))
      }
    }

    defi
  }

  def findDefinition(defType: key): Option[value] = {
    var bestMatch = 0
    var defi: Option[value] = None

    for {
      k <- catalogs.keys
      if k.getClass == defType.getClass
      if k.ident.length >= defType.ident.length
    } {
      var curMatch = 0
      for {
        (s, i) <- k.ident.zipWithIndex
        if i < defType.ident.length
        if i == curMatch
        if s == defType.ident(i)
      } curMatch += 1

      if (
        curMatch > bestMatch &&
        curMatch >= defType.ident.length
      ) {
        defi = Some(catalogs(k))
        bestMatch = curMatch
      }
    }

    defi
  }

  def mergeNewDefinition(
      incoming: Map[DefinitionType, DefinitionTrait]
  ): Unit = {
    // check for duplicates
    for (i <- catalogs.keys) {
      if (incoming.contains(i)) {
        println(
          s"WARN: Duplicate definition name ${i.ident.mkString(".")} detected"
        )
      }
    }
    catalogs ++= incoming
  }
}

object DefinitionCatalog {
  def fromFile(
      fileName: String,
      defaultMap: Map[String, Variant]
  ): Option[Seq[DefinitionTrait]] = {
    val filePath = Project.catalogPath.resolve(fileName)

    // println(s"Reading $fileName catalog")

    if (!Files.exists(filePath.toAbsolutePath)) {
      println(s"$fileName catalog at $filePath not found")
      return None
    }

    Project.pushCatalogPath(filePath.getParent)

    val source = Utils.readYaml(filePath)
    val result = parse(fileName, source, defaultMap)
    Project.popCatalogPath()
    result
  }

  private def parse(
      name: String,
      parsed: Map[String, Variant],
      defaultMap: Map[String, Variant]
  ): Option[Seq[DefinitionTrait]] = {

    if (parsed.contains("instance")) {
      println(
        s"$name contains an Instance which are not allowed in definitions"
      )
      return None
    };

    val defaults =
      if (parsed.contains("defaults"))
        Utils.mergeAintoB(Utils.toTable(parsed("defaults")), defaultMap)
      else defaultMap

    val defs = ArrayBuffer[DefinitionTrait]()
    if (parsed.contains("definition")) {
      val tdef = Utils.toArray(parsed("definition"))
      for (defi <- tdef) defs += Definition(defi, defaults)
    }

    if (parsed.contains("include")) {
      val tincs = Utils.toArray(parsed("include"))
      for (include <- tincs) {
        val table = Utils.toTable(include)
        val name = Paths.get(Utils.toString(table("resource")))
        Project.pushCatalogPath(name)
        val cat = DefinitionCatalog.fromFile(s"${name.getFileName}", defaults)
        cat match {
          case Some(value) => defs ++= value
          case None        =>
        }
        Project.popCatalogPath()
      }
    }

    if (parsed.contains("resources")) {
      val resources = Utils.lookupArray(parsed, "resources")
      for (resource <- resources) {
        val name = Utils.toString(resource)
        val cat = DefinitionCatalog.fromFile(s"$name", defaults)
        cat match {
          case Some(value) => defs ++= value
          case None        =>
        }
      }
    }
    val identArray = defs.map(_.defType.ident.mkString(".")).toArray
    for (i <- 0 until identArray.length)
      for (j <- i + 1 until identArray.length) {
        if (identArray(i) == identArray(j)) {
          println(s"WARN ${identArray(i)} already exists in catalog")
        }
      }

    Some(defs.toSeq)
  }

}
