package overlord.Instances

import overlord.Connections.Connection

import java.nio.file.Path
import overlord.Definitions._
import overlord.Gateware.{Gateware, Parameter, Port}
import overlord.{DefinitionCatalog, Utils}
import toml.Value

import scala.collection.mutable

trait Instance {
	val ident     : String
	val definition: DefinitionTrait
	val attributes: Map[String, Value]

	private val splitIdent           = ident.split('.')
	private val splitIdentWidthIndex = splitIdent.zipWithIndex

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

	def copyMutate[A <: Instance](nid: String,
	                              nattribs: Map[String, Value]): Instance

	def count: Int = Utils.lookupInt(attributes, "count", 1)

	def shared: Boolean = attributes.contains("shared")

	def isGateware: Boolean = definition.gateware.nonEmpty

	lazy val phase2Ports: Map[String, Port] =
		definition.ports ++ (if (definition.gateware.nonEmpty)
			definition.gateware.get.ports.toMap else Map())

	def getPort(lastName: String): Option[Port] = {
		if (phase2Ports.contains(lastName))
			Some(phase2Ports(lastName))
		else None
	}
	def getPorts: Map[String,Port] = phase2Ports

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
	def apply(parsed: Value,
	          defaults: Map[String, Value],
	          catalogs: DefinitionCatalog): Option[Instance] = {

		val table = Utils.toTable(parsed)

		if (!table.contains("type")) {
			println(s"$parsed doesn't have a type")
			return None
		}

		val defTypeString = Utils.toString(table("type"))
		val name          = Utils.lookupString(table, "name", defTypeString)

		val attribs: Map[String, Value] = (defaults ++ table.filter(
			_._1 match {
				case "type" | "name" => false
				case _               => true
			}))

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

		defi.createInstance(name, attribs)

	}

	private def definitionFrom(catalogs: DefinitionCatalog,
	                           table: Map[String, Value],
	                           defTypeString: String,
	                           name: String,
	                           attribs: Map[String, Value],
	                           defType: DefinitionType)
	: Option[Definition] = {
		if (table.contains("gateware")) {
			val path = Path.of(
				Utils.lookupString(table, "gateware", s"$name/$name" + s".toml"))

			Gateware(defTypeString, path) match {
				case Some(gw) =>
					val result = Definition(defType, attribs,
					                        gw.ports.toMap,
					                        gw.parameters.toMap,
					                        None,
					                        Some(gw),
					                        None)
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
				println(s"${defType} not found in any catalogs")
				None
		}
	}
}