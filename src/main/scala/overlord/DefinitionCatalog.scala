package overlord

import ikuy_utils._

import java.nio.file.{Files, Path}
import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer

class DefinitionCatalog {
	type key = DefinitionType
	type value = DefinitionTrait
	type keyStore = mutable.HashMap[key, value]

	val catalogs: keyStore = mutable.HashMap()

	def FindDefinition(defType: key): Option[value] = {
		var bestMatch           = 0
		var defi: Option[value] = None

		for {k <- catalogs.keys
		     if k.getClass == defType.getClass
		     if k.ident.length >= defType.ident.length} {
			var curMatch = 0
			for {(s, i) <- k.ident.zipWithIndex
			     if i < defType.ident.length
			     if i == curMatch
			     if s == defType.ident(i)
			     } curMatch += 1

			if (curMatch > bestMatch &&
			    curMatch >= defType.ident.length) {
				defi = Some(catalogs(k))
				bestMatch = curMatch
			}
		}

		defi
	}

}

object DefinitionCatalog {
	def fromFile(name: String,
	             spath: Path,
	             defaultMap: Map[String,Variant]): Option[Seq[DefinitionTrait]] = {
		println(s"Reading $name catalog")

		val path = spath.resolve(s"$name.toml")

		if (!Files.exists(path.toAbsolutePath)) {
			println(s"$name catalog at $path not found")
			return None
		}

		val source = Utils.readToml(name, path, getClass)
		parse(source, spath, defaultMap)
	}
	private def parse(parsed: Map[String, Variant],
	                  spath: Path,
	                  defaultMap: Map[String,Variant]): Option[Seq[DefinitionTrait]] = {

		val path = if (parsed.contains("path")) {
			spath.resolve(Utils.toString(parsed("path")))
		} else spath

		val defaults = if (parsed.contains("defaults"))
			Utils.mergeAintoB(Utils.toTable(parsed("defaults")), defaultMap)
		else defaultMap

		val defs = ArrayBuffer[DefinitionTrait]()
		if (parsed.contains("definition")) {
			val tdef = Utils.toArray(parsed("definition"))
			for (defi <- tdef) defs += Definition(defi, path, defaults)
		}

		if (parsed.contains("include")) {
			val tincs = Utils.toArray(parsed("include"))
			for (include <- tincs) {
				val table = Utils.toTable(include)
				val name  = Utils.toString(table("resource"))
				val cat   = DefinitionCatalog.fromFile(s"$name", path, defaults)
				cat match {
					case Some(value) => defs ++= value
					case None        =>
				}
			}
		}

		if (parsed.contains("resources")) {
			val resources = Utils.lookupArray(parsed, "resources")
			for (resource <- resources) {
				val name = Utils.toString(resource)
				val cat  = DefinitionCatalog.fromFile(s"$name", path, defaults)
				cat match {
					case Some(value) => defs ++= value
					case None        =>
				}
			}
		}

		Some(defs.toSeq)
	}
}