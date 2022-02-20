package overlord.Instances

import overlord.Chip.{HardwareDefinition, Port, RegisterBank, RegisterList}
import overlord.ChipDefinitionTrait

import scala.collection.mutable

trait ChipInstance extends InstanceTrait {
	override def definition: ChipDefinitionTrait

	val isHardware: Boolean = definition.isInstanceOf[HardwareDefinition]

	lazy val ports: mutable.HashMap[String, Port] =
		mutable.HashMap[String, Port](definition.ports.toSeq: _*)

	val instanceRegisterBanks: mutable.ArrayBuffer[RegisterBank] = mutable.ArrayBuffer()
	val instanceRegisterLists: mutable.ArrayBuffer[RegisterList] = mutable.ArrayBuffer()

	private lazy val instanceParameterKeys: mutable.HashSet[String] = mutable.HashSet()

	def mergePort(key: String, port: Port): Unit =
		ports.updateWith(key) {
			case Some(_) => Some(port)
			case None => Some(port)
		}

	def mergeParameterKey(key: String): Unit = instanceParameterKeys += key

	val splitIdent: Array[String] = ident.split('.')
	private val splitIdentWidthIndex = splitIdent.zipWithIndex

	lazy val registerBanks: Seq[RegisterBank] =
		if(definition.registers.nonEmpty)
			definition.registers.get.banks ++ instanceRegisterBanks
		else instanceRegisterBanks.toSeq

	lazy val registerLists: Seq[RegisterList] =
		if(definition.registers.nonEmpty)
			definition.registers.get.lists ++ instanceRegisterLists
		else instanceRegisterLists.toSeq

	lazy val parameterKeys: Set[String] = instanceParameterKeys.toSet

	def copyMutate[A <: ChipInstance](nid: String): ChipInstance

	def getPort(lastName: String): Option[Port] =
		if (ports.contains(lastName)) Some(ports(lastName)) else None

	def getMatchNameAndPort(name: String): (Option[String], Option[Port]) = {
		val nameWithoutBits = name.split('[').head

		def WildCardMatch(nameId: Array[String], instanceId: Array[String]) = {
			// wildcard match
			val is = for ((id, i) <- instanceId.zipWithIndex) yield
				if ((i < nameId.length) && (id == "_" || id == "*"))
					nameId(i)
				else id

			val ms = for ((id, i) <- nameId.zipWithIndex) yield
				if ((i < is.length) && (id == "_" || id == "*")) is(i)
				else id

			if (ms sameElements is)
				(Some(ms.mkString(".")), getPort(ms(0)))
			else {
				val msv = ms.reverse
				val mp  = msv.head
				val tms = if (msv.length > 1) msv.tail.reverse else msv

				val port = getPort(mp)
				if ((is sameElements tms) && port.nonEmpty)
					(Some(s"${ms.mkString(".")}"), port)
				else (None, None)
			}
		}

		if (name == ident)
			(Some(name), getPort(name.split('.').last))
		else if (nameWithoutBits == ident)
			(Some(nameWithoutBits), getPort(nameWithoutBits.split('.').last))
		else {
			val match0 = WildCardMatch(nameWithoutBits.split('.'), ident.split('.'))
			if (match0._1.isDefined) return match0
			WildCardMatch(nameWithoutBits.split('.'), definition.defType.ident.toArray)
		}
	}
}

