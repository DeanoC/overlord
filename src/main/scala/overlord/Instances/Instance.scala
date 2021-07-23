package overlord.Instances

import overlord.Connections.Connection

import java.nio.file.Path
import overlord.Definitions._
import overlord.Gateware.{Gateware, Port}
import overlord.DefinitionCatalog
import ikuy_utils._
import overlord.Software.{RegisterBank, RegisterList}

import scala.collection.mutable

trait Instance {
	val ident: String

	val replicationCount: Int     = 1 // TODO revisit this
	val shared          : Boolean = false


	val instanceRegisterBanks: mutable.ArrayBuffer[RegisterBank] = mutable.ArrayBuffer()
	val instanceRegisterLists: mutable.ArrayBuffer[RegisterList] = mutable.ArrayBuffer()
	val instanceDocs         : mutable.ArrayBuffer[String]       = mutable.ArrayBuffer()

	private lazy val instancePorts: mutable.HashMap[String, Port] = {
		mutable.HashMap[String, Port](definition.ports.toSeq: _*) ++ {
			if (definition.gateware.nonEmpty)
				definition.gateware.get.ports
			else mutable.HashMap[String, Port]()
		}
	}

	private lazy val instanceAttributes: mutable.HashMap[String, Variant] = {
		mutable.HashMap[String, Variant](definition.attributes.toSeq: _*) ++ {
			if (definition.gateware.nonEmpty)
				definition.gateware.get.parameters
			else Map[String, Variant]()
		}
	}
	private lazy val instanceParameterKeys: mutable.HashSet[String] = mutable.HashSet()

	def mergeParameter(attribs: Map[String, Variant], key: String): Unit = {
		if (attribs.contains(key)) {
			// overwrite existing or add it
			instanceAttributes.updateWith(key) {
				case Some(_) => Some(attribs(key))
				case None    => Some(attribs(key))
			}
		}
	}

	def mergePort(key: String, port: Port): Unit =
		instancePorts.updateWith(key) {
			case Some(_) => Some(port)
			case None => Some(port)
		}

	def mergeParameterKey(key: String): Unit = instanceParameterKeys += key

	private val splitIdent           = ident.split('.')
	private val splitIdentWidthIndex = splitIdent.zipWithIndex

	def definition: DefinitionTrait

	def ports: Map[String, Port] = instancePorts.toMap

	def attributes: Map[String, Variant] = instanceAttributes.toMap

	def registerLists: Seq[RegisterList] = definition.registerLists ++
	                                       instanceRegisterLists

	def registerBanks: Seq[RegisterBank] = definition.registerBanks ++
	                                       instanceRegisterBanks

	def docs: Seq[String] = definition.docs ++ instanceDocs

	def parameterKeys: Set[String] = instanceParameterKeys.toSet

	def hardware: Option[HardwareTrait] = definition.hardware

	def copyMutate[A <: Instance](nid: String): Instance

	def isHardware: Boolean = !isGateware
	def isGateware: Boolean = definition.gateware.nonEmpty

	def getPort(lastName: String): Option[Port] =
		if (ports.contains(lastName)) Some(ports(lastName)) else None

	def getMatchNameAndPort(a: String): (Option[String], Option[Port]) = {
		val withoutBits = a.split('[').head
		if (a == ident)
			(Some(a), getPort(a.split('.').last))
		else if (withoutBits == ident)
			(Some(withoutBits), getPort(withoutBits.split('.').last))
		else {
			val splitWithoutBits = withoutBits.split('.')

			// wildcard match both ident and match
			val is = for ((id, i) <- splitIdentWidthIndex) yield
				if ((i < splitWithoutBits.length) && (id == "_" || id == "*"))
					splitWithoutBits(i)
				else id

			val ms =
				for ((id, i) <- withoutBits.split('.').zipWithIndex) yield
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
	}
}

trait Container {
	val children: Seq[Instance]
	val physical: Boolean

	lazy val flatChildren: Seq[Instance] =
		children.filter(_.isInstanceOf[Container])
			.map(_.asInstanceOf[Container]).flatMap(_.flatChildren) ++ children

	def copyMutateContainer(copy: MutContainer): Container
}

class MutContainer(var children: mutable.Seq[Instance] = mutable.Seq(),
                   var connections: mutable.Seq[Connection] = mutable.Seq())

object Instance {
	def apply(parsed: Variant,
	          defaults: Map[String, Variant],
	          catalogs: DefinitionCatalog): Option[Instance] = {

		val table = Utils.toTable(parsed)

		if (!table.contains("type")) {
			println(s"$parsed doesn't have a type")
			return None
		}

		val defTypeString    = Utils.toString(table("type"))
		val name             = Utils.lookupString(table, "name", defTypeString)
		val replicationCount = Utils.lookupInt(table, "count", 1)
		val shared           = Utils.lookupBoolean(table, "shared", or = false)

		val attribs: Map[String, Variant] = defaults ++ table.filter(
			_._1 match {
				case "type" | "name"    => false
				case "shared" | "count" => false
				case _                  => true
			})

		val defType = Definition.toDefinitionType(defTypeString)

		val defi = catalogs.FindDefinition(defType) match {
			case Some(d) => d
			case None    => definitionFrom(catalogs,
			                               table,
			                               defTypeString,
			                               name,
			                               attribs,
			                               defType) match {
				case Some(value) => value
				case None        =>
					println(s"No definition found or could be create $name $defType")
					return None
			}
		}

		defi.createInstance(name, attribs) match {
			case Some(i) =>
				//				i.replicationCount = replicationCount
				//				i.shared = shared
				Some(i)
			case None    => None
		}

	}

	private def definitionFrom(catalogs: DefinitionCatalog,
	                           table: Map[String, Variant],
	                           defTypeString: String,
	                           name: String,
	                           attribs: Map[String, Variant],
	                           defType: DefinitionType)
	: Option[Definition] = {
		if (table.contains("gateware")) {
			val path = Path.of(
				Utils.lookupString(table, "gateware", s"$name/$name" + s".toml"))

			Gateware(defTypeString, path) match {
				case Some(gw) =>
					val result = Definition(defType, attribs,
					                        gw.ports.toMap,
					                        Some(gw))
					catalogs.catalogs += (defType -> result)
					Some(result)

				case None =>
					println(s"gateware $path not found")
					None
			}
		} else defType match {
			/*		case RamDefinitionType(ident)      =>
						case CpuDefinitionType(ident)      =>
						case NxMDefinitionType(ident)      =>
						case StorageDefinitionType(ident)  =>
						case SocDefinitionType(ident)      =>
						case BridgeDefinitionType(ident)   =>
						case NetDefinitionType(ident)      =>
						case OtherDefinitionType(ident)    =>
						case PortDefinitionType(ident)     =>
						case ConstantDefinitionType(ident) =>
			*/
			case BoardDefinitionType(_) |
			     ClockDefinitionType(_) |
			     PinGroupDefinitionType(_) =>
				val result = Definition(defType, attribs)
				catalogs.catalogs += (defType -> result)
				Some(result)
			case _                         =>
				println(s"$defType not found in any catalogs")
				None
		}
	}
}