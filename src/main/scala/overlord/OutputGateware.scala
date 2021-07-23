package overlord

import ikuy_utils.{BigIntV, Variant}
import overlord.Connections.{ConnectedConstant, Connection,
	ConstantConnectionType}
import overlord.Definitions.GatewareTrait
import overlord.Gateware.GatewareAction.GatewareAction
import overlord.Instances.{BusInstance, Instance, MutContainer, RamInstance}

import java.nio.file.Path
import scala.collection.mutable

object OutputGateware {
	def apply(top: MutContainer,
	          gatePath: Path,
	          phase: GatewareAction => Boolean): Unit = {
		Game.pathStack.push(gatePath.toRealPath())

		for {(instance, gateware) <-
			     top.children.filter(_.definition.gateware.nonEmpty)
				     .map(g => (g, g.definition.gateware.get))} {

			OutputGateware.executePhase(instance, gateware,
			                            top.connections.toSeq, phase)
		}
	}

	def executePhase(instance: Instance,
	                 gateware: GatewareTrait,
	                 connections: Seq[Connection],
	                 phase: GatewareAction => Boolean): Unit = {

		val backupStack = Game.pathStack.clone()
		for {action <- gateware.actions.filter(phase(_))} {

			val conParameters = connections
				.filter(_.isUnconnected)
				.map(_.asUnconnected)
				.filter(_.isConstant).map(c => {
				val constant = c.connectionType.asInstanceOf[ConstantConnectionType]
				val name     = c.secondFullName.split('.').lastOption match {
					case Some(value) => value
					case None        => c.secondFullName
				}
				mutable.Map[String, Variant](name -> constant.constant)
			}).fold(mutable.HashMap[String, Variant]())((o, n) => o ++ n)

			val instanceSpecificParameters = instance match {
				case bus: BusInstance =>
					Map[String, Variant]("consumers" -> bus.consumersVariant)
				case _                => Map[String, Variant]()
			}

			val parameters = instance.attributes ++
			                 conParameters ++
			                 instanceSpecificParameters

			val merged = connections.filter(
				_.isInstanceOf[ConnectedConstant])
				.map(_.asInstanceOf[ConnectedConstant])
				.filter(c => instance.parameterKeys.contains(c.secondFullName))
				.map(_.asParameter)
				.fold(parameters)((o, n) => o ++ n)

			action.execute(instance, merged, Game.pathStack.top)
		}

		Game.pathStack = backupStack
	}
}
