package overlord.Instances

import ikuy_utils.{Utils, Variant}
import overlord.Chip.ChipDefinition
import overlord.Interfaces.QueryInterface
import overlord.Software.SoftwareDefinition
import overlord.{DefinitionCatalog, DefinitionTrait, DefinitionType, Game}

import java.nio.file.Path
import scala.collection.mutable

trait InstanceTrait extends QueryInterface {
	lazy val attributes: mutable.HashMap[String, Variant] = mutable.HashMap[String, Variant](definition.attributes.toSeq: _*)
	val name: String
	val sourcePath: Path = Game.instancePath.toAbsolutePath

	def definition: DefinitionTrait

	def mergeAllAttributes(attribs: Map[String, Variant]): Unit =
		attribs.foreach(a => mergeAttribute(attribs, a._1))

	def mergeAttribute(attribs: Map[String, Variant], key: String): Unit = {
		if (attribs.contains(key)) {
			// overwrite existing or add it
			attributes.updateWith(key) {
				case Some(_) => Some(attribs(key))
				case None    => Some(attribs(key))
			}
		}
	}
}

object Instance {
	def apply(parsed: Variant,
	          defaults: Map[String, Variant],
	          catalogs: DefinitionCatalog): Option[InstanceTrait] = {

		val table = Utils.toTable(parsed)

		if (!table.contains("type")) {
			println(s"$parsed doesn't have a type")
			return None
		}

		val defTypeString = Utils.toString(table("type"))
		val name          = Utils.lookupString(table, "name", defTypeString)

		val attribs: Map[String, Variant] = defaults ++ table.filter(
			_._1 match {
				case "type" | "name" => false
				case _               => true
			})

		val defType = DefinitionType(defTypeString)

		val defi = catalogs.findDefinition(defType) match {
			case Some(d) => d
			case None    =>
				definitionFrom(catalogs, Game.projectPath, table, defType) match {
					case Some(value) => value
					case None        =>
						println(s"No definition found or could be create $name $defType")
						return None
				}
		}

		defi.createInstance(name, attribs) match {
			case Some(i) => Some(i)
			case None    => None
		}
	}

	private def definitionFrom(catalogs: DefinitionCatalog,
	                           path: Path,
	                           table: Map[String, Variant],
	                           defType: DefinitionType)
	: Option[DefinitionTrait] = {

		if (table.contains("gateware")) {
			val result = ChipDefinition(table, path)
			result match {
				case Some(value) =>
					catalogs.catalogs += (defType -> value)
					result
				case None        =>
					println(s"$defType gateware was invalid")
					None
			}
		} else if (table.contains("software")) {
			val result = SoftwareDefinition(table, path)
			result match {
				case Some(value) =>
					catalogs.catalogs += (defType -> value)
					result
				case None        =>
					println(s"$defType software was invalid")
					None
			}
		} else {
			println(s"$defType not found in any catalogs")
			None
		}
	}
}