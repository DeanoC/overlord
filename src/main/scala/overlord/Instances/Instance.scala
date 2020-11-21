package overlord.Instances

import java.nio.file.Path
import overlord.Definitions._
import overlord.Gateware.Gateware
import overlord.{DefinitionCatalog, Utils}
import toml.Value

trait Instance {
	val ident     : String
	val definition: DefinitionTrait
	val attributes: Map[String, Value]

	private val splitIdent           = ident.split('.')
	private val splitIdentWidthIndex = splitIdent.zipWithIndex

	def hasPortsOrParameters: Boolean =
		definition.defType.hasPortsOrParameters

	def getMatchName(a: String): Option[String] = {
		val withoutBits = a.split('[').head
		if (a == ident) Some(a)
		else if (withoutBits == ident) Some(withoutBits)
		else {

			// wildcard match both ident and match
			val is = for ((id, i) <- splitIdentWidthIndex) yield {
				if ((i < withoutBits.length) && (id == "_" || id == "*"))
					withoutBits(i)
				else id
			}
			val ms = for ((id, i) <- withoutBits.split('.').zipWithIndex) yield {
				if ((i < is.length) && (id == "_" || id == "*")) is(i)
				else id
			}

			if (ms sameElements is) Some(ms.mkString("."))
			else if (definition.defType.hasPortsOrParameters) {
				val msv = ms.reverse
				val mp  = msv.head
				val tms = if (msv.length > 1) msv.tail.reverse else msv

				if ((is sameElements tms) &&
				    definition.defType.portsOrParameters.contains(mp))
					Some(s"${ms.mkString(".")}")
				else None
			} else None

		}
	}

	def copyMutate[A <: Instance](nid: String,
	                              nattribs: Map[String, Value]): Instance

	def count: Int = Utils.lookupInt(attributes, "count", 1)

	def shared: Boolean = attributes.contains("shared")

	def isGateware: Boolean = definition.gateware.nonEmpty
}

object Instance {
	def apply(parsed: Value,
	          catalogs: DefinitionCatalog): Option[Instance] = {

		val table = Utils.toTable(parsed)

		if (!table.contains("type")) {
			println(s"$parsed doesn't have a type")
			return None
		}

		val defTypeString = Utils.toString(table("type"))
		val name          = Utils.lookupString(table, "name", defTypeString)

		val attribs: Map[String, Value] = table.filter(
			_._1 match {
				case "type" | "name" => false
				case _               => true
			})

		val defType = Definition.toDefinitionType(defTypeString, Seq())

		val defi = catalogs.FindDefinition(defType) match {
			case Some(d) => d
			case None    =>
				if (table.contains("gateware")) {
					val path = Path.of(
						Utils.lookupString(table, "gateware", s"$name/$name" + s".toml"))

					Gateware(defTypeString, path) match {
						case Some(gw) =>
							val dt     =
								Definition.toDefinitionType(defTypeString,
								                            gw.ports ++
								                            gw.parameters.map(_._1).toSeq)
							val result = Definition(dt, attribs, None, Some(gw))
							catalogs.catalogs += (dt -> result)
							result

						case None =>
							println(s"gateware $path not found")
							return None
					}
				} else {
					println(s"${defType} not found in any catalogs")
					return None
				}
		}

		defType match {
			case _: RamDefinitionType      =>
				Some(RamInstance(name, defi, attribs))
			case _: CpuDefinitionType      =>
				Some(CpuInstance(name, defi, attribs))
			case _: NxMDefinitionType      =>
				Some(NxMInstance(name, defi, attribs))
			case _: StorageDefinitionType  =>
				Some(StorageInstance(name, defi, attribs))
			case _: SocDefinitionType      =>
				Some(SocInstance(name, defi, attribs))
			case _: BridgeDefinitionType   =>
				Some(BridgeInstance(name, defi, attribs))
			case _: NetDefinitionType      =>
				Some(NetInstance(name, defi, attribs))
			case _: OtherDefinitionType    =>
				Some(OtherInstance(name, defi, attribs))
			case _: PortDefinitionType     => None
			case _: ClockDefinitionType    => None
			case _: ConstantDefinitionType => None
		}
	}
}