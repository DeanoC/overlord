package com.deanoc.overlord

import java.nio.file.{Files, Path, Paths}
import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer
import sys.process._

import com.deanoc.overlord.utils.{Utils, Variant}
import com.deanoc.overlord.utils.Logging
import com.deanoc.overlord.definitions.ComponentDefinition

import com.deanoc.overlord.Overlord
import definitions.DefinitionTrait
import definitions.DefinitionType

class DefinitionCatalog extends Logging {
  type key = DefinitionType
  type value = DefinitionTrait
  type keyStore = mutable.HashMap[key, value]

  val definitions: keyStore = mutable.HashMap()

  def findDefinition(name: String): Option[value] = {
    val ident = name.split('.').toList
    var defi: Option[value] = None

    for {
      k <- definitions.keys
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
        defi = Some(definitions(k))
      }
    }

    defi
  }

  def findDefinition(defType: key): Option[value] = {
    var bestMatch = 0
    var defi: Option[value] = None

    for {
      k <- definitions.keys
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
        defi = Some(definitions(k))
        bestMatch = curMatch
      }
    }

    defi
  }

  def mergeNewDefinition(
      incoming: Seq[DefinitionTrait]
  ): Unit = {
    val withType : Map[DefinitionType, DefinitionTrait] = incoming.map { t =>
      DefinitionType(t.defType.ident.mkString(".")) -> t
    }.toMap

    // check for duplicates
    for (i <- definitions.keys) {
      if (withType.contains(i)) {
        warn(
          s"Duplicate definition name ${i.ident.mkString(".")} detected"
        )
      }
    }
    definitions ++= withType
  }
  
  /**
   * Adds a ComponentDefinition to the catalog.
   *
   * @param component The Component to create a ComponentDefinition from
   * @return This DefinitionCatalog with the ComponentDefinition added
   */
  def addComponentDefinition(component: Component): DefinitionCatalog = {
    val componentDef = ComponentDefinition(component)
    definitions += (componentDef.defType -> componentDef)
    this
  }
}