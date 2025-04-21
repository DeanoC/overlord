package com.deanoc.overlord

import scala.language.implicitConversions

import java.nio.file.Path

import com.deanoc.overlord.connections._
import com.deanoc.overlord.connections.ConnectionDirection
import com.deanoc.overlord.instances.{ChipInstance, Container}
import com.deanoc.overlord.interfaces.UnconnectedLike
import com.deanoc.overlord.Overlord
import com.deanoc.overlord.utils._
import definitions.GatewareDefinitionTrait
import com.deanoc.overlord.config.ConfigPaths
object OutputGateware {
  def apply(
      top: Container,
      gatePath: Path,
      constants: Seq[Constant],
      phase: Int
  ): Unit = {
    ConfigPaths.pushCatalogPath(gatePath)

    top.children
      .collect { case c: ChipInstance => c }
      .filter(_.isGateware)
      .foreach(executePhase(_, top.unconnected, constants, phase))

    ConfigPaths.popCatalogPath()
  }

  private def executePhase(
      instance: ChipInstance,
      unconnections: Seq[UnconnectedLike],
      constants: Seq[Constant],
      phase: Int
  ): Unit = {
    // Get the gateware definition with explicit type casting
    val gateware = instance.definition.asInstanceOf[GatewareDefinitionTrait]

    /* TODO: Process actions
      
    // Get the actions from the gateware definition
    val actions = gateware.actionsFile.actions

    // Extract parameters with explicit dependency injection
    val parameters = extractParameters(
      instance,
      unconnections,
      constants,
      gateware.parameters
    )

    // Add parameters to the instance's parameter table
    instance.finalParameterTable.addAll(for ((k, v) <- parameters) yield k -> v)

    // Execute actions for the current phase with explicit dependency injection
    for { action <- actions.filter(_.phase == phase) }
      action.execute(instance, instance.finalParameterTable.toMap)
      */
  }

  private def extractParameters(
      instance: ChipInstance,
      unconnections: Seq[UnconnectedLike],
      constants: Seq[Constant],
      gatewareParameters: Map[String, Variant]
  ): Map[String, Variant] = {
    val busParameters = Map[String, Variant](
      "buses" -> ArrayV(
        unconnections
          .collect { case b: UnconnectedBus =>
            val isFirst = b.firstFullName == instance.name
            val isSecond = b.secondFullName == instance.name
            if (isFirst || isSecond) {
              Array(TableV {
                Array(
                  ("name", StringV(b.supplierBusName)),
                  ("consumer_name", StringV(b.consumerBusName)),
                  ("protocol", StringV(b.busProtocol)),
                  (
                    "supplier",
                    if (
                      b.direction == ConnectionDirection.FirstToSecond && isFirst
                    )
                      BooleanV(true)
                    else BooleanV(false)
                  )
                ).toMap
              })
            } else Array[TableV]()
          }
          .fold(Array[Variant]())((o, n) => o ++ n)
          .map(_.asInstanceOf[Variant])
      )
    )

    val instanceParameters =
      if (instance.attributes.contains("parameters")) {
        Utils
          .toArray(instance.attributes("parameters"))
          .map { p =>
            val t = Utils.toTable(p)
            Map[String, Variant](
              Utils.lookupString(t, "name", "NO_NAME") -> t("value")
            )
          }
          .fold(Map())((o, n) => o ++ n)
      } else Map()

    val connectedConstants: Map[String, Variant] = constants.flatMap { c =>
      val name = s"${c.parameter.name}"
      c.parameter.parameterType match {
        case ConstantParameterType(value) => Some(name, value)
        case FrequencyParameterType(freq) =>
          Some(name, StringV(freq.toString + " Mhz"))
      }
    }.toMap

    instanceParameters ++ busParameters ++ connectedConstants
  }
}
