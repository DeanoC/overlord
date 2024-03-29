package overlord

import ikuy_utils._
import overlord.Connections._
import overlord.Instances.{ChipInstance, Container}
import overlord.Interfaces.UnconnectedLike

import java.nio.file.Path

object OutputGateware {
	def apply(top: Container,
	          gatePath: Path,
	          constants: Seq[Constant],
	          phase: Int): Unit = {
		Game.pushCatalogPath(gatePath)

		top.children
			.collect { case c: ChipInstance => c }
			.filter(_.isGateware)
			.foreach(executePhase(_, top.unconnected, constants, phase))

		Game.popCatalogPath()
	}

	private def executePhase(instance: ChipInstance,
	                         unconnections: Seq[UnconnectedLike],
	                         constants: Seq[Constant],
	                         phase: Int): Unit = {

		val gateware = instance.definition.asInstanceOf[GatewareDefinitionTrait]
		val actions  = gateware.actionsFile.actions

		val parameters = gateware.parameters ++ extractParameters(instance, unconnections, constants)

		instance.finalParameterTable.addAll(for ((k, v) <- parameters) yield k -> v)

		for {action <- actions.filter(_.phase == phase)} action.execute(instance, instance.finalParameterTable.toMap)
	}

	private def extractParameters(instance: ChipInstance, unconnections: Seq[UnconnectedLike], constants: Seq[Constant]) = {
		val busParameters = Map[String, Variant]("buses" -> ArrayV(unconnections.collect {
			case b: UnconnectedBus =>
				val isFirst  = b.firstFullName == instance.name
				val isSecond = b.secondFullName == instance.name
				if (isFirst || isSecond) {
					Array(TableV {
						Array(
							("name", StringV(b.supplierBusName)),
							("consumer_name", StringV(b.consumerBusName)),
							("protocol", StringV(b.busProtocol)),
							("supplier", if (b.direction == FirstToSecondConnection() && isFirst) BooleanV(true) else BooleanV(false))
							).toMap
					})
				} else Array[TableV]()
		}.fold(Array())((o, n) => o ++ n).map(_.asInstanceOf[Variant])))

		val instanceParameters =
			if (instance.attributes.contains("parameters")) {
				Utils.toArray(instance.attributes("parameters")).map { p =>
					val t = Utils.toTable(p)
					Map[String, Variant](Utils.lookupString(t, "name", "NO_NAME") -> t("value"))
				}.fold(Map())((o, n) => o ++ n)
			} else Map()

		val connectedConstants: Map[String, Variant] = constants.flatMap { c =>
			val name = s"${c.parameter.name}"
			c.parameter.parameterType match {
				case ConstantParameterType(value) => Some(name, value)
				case FrequencyParameterType(freq) => Some(name, StringV(freq.toString + " Mhz"))
				case _                            => None
			}
		}.toMap

		instanceParameters ++ busParameters ++ connectedConstants
	}
}
