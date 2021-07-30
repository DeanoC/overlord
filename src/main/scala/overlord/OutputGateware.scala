package overlord

import ikuy_utils.Variant
import overlord.Connections.{ConnectedConstant, Connection, ConstantConnectionType}
import overlord.Instances.{BusInstance, ChipInstance, MutContainer}

import java.nio.file.Path

object OutputGateware {
	def apply(top: MutContainer,
	          gatePath: Path,
	          phase: Int): Unit = {
		Game.pathStack.push(gatePath.toRealPath())

		for (instance <- top.children
			.filter(_.definition.isInstanceOf[GatewareDefinitionTrait])
			.map(_.asInstanceOf[ChipInstance])) {
			OutputGateware.executePhase(instance, top.connections.toSeq, phase)
		}
	}

	def executePhase(instance: ChipInstance,
	                 connections: Seq[Connection],
	                 phase: Int): Unit = {
		val backupStack = Game.pathStack.clone()

		val gateware = instance.definition.asInstanceOf[GatewareDefinitionTrait]
		val actions = gateware.actionsFile.actions

		for {
			action <- actions.filter(_.phase == phase)
		} {
			val conParameters = connections
				.filter(_.isUnconnected)
				.map(_.asUnconnected)
				.filter(_.isConstant).map(c => {
				val constant = c.connectionType.asInstanceOf[ConstantConnectionType]
				val name     = c.secondFullName.split('.').lastOption match {
					case Some(value) => value
					case None        => c.secondFullName
				}
				Map[String, Variant](name -> constant.constant)
			}).fold(Map[String, Variant]())((o, n) => o ++ n)

			val instanceSpecificParameters = instance match {
				case bus: BusInstance =>
					Map[String, Variant]("consumers" -> bus.consumersVariant)
				case _                => Map[String, Variant]()
			}

			val parameters = gateware.parameters ++
			                 conParameters ++
			                 instanceSpecificParameters

			action.execute(instance, parameters, Game.pathStack.top)
		}

		Game.pathStack = backupStack
	}
}
