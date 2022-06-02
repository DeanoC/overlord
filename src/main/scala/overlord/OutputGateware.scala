package overlord

import ikuy_utils.Variant
import overlord.Connections.UnConnectedConstant
import overlord.Instances.{ChipInstance, Container}
import overlord.Interfaces.UnConnectedLike

import java.nio.file.Path

object OutputGateware {
	def apply(top: Container,
	          gatePath: Path,
	          phase: Int): Unit = {
		Game.pushCatalogPath(gatePath)

		top.children
			.collect { case c: ChipInstance => c }
			.filter(_.isGateware)
			.foreach(executePhase(_, top.unconnected, phase))

		Game.popCatalogPath()
	}

	private def executePhase(instance: ChipInstance,
	                         unconnections: Seq[UnConnectedLike],
	                         phase: Int): Unit = {

		val gateware = instance.definition.asInstanceOf[GatewareDefinitionTrait]
		val actions  = gateware.actionsFile.actions

		for {
			action <- actions.filter(_.phase == phase)
		} {
			val conParameters = unconnections
				.filter(_.isInstanceOf[UnConnectedConstant]).map(c => {
				val name = c.secondFullName.split('.').lastOption match {
					case Some(value) => value
					case None        => c.secondFullName
				}
				Map[String, Variant](name -> c.asInstanceOf[UnConnectedConstant].constant)
			}).fold(Map[String, Variant]())((o, n) => o ++ n)


			val parameters = gateware.parameters ++ conParameters

			instance.mergeAllAttributes(parameters)

			val parametersTbl = for ((k, v) <- parameters) yield k -> v

			action.execute(instance, parametersTbl)
		}
	}
}
