package overlord

import java.nio.file.{Files, Path}
import overlord.Definitions.{Definition, DefinitionTrait, DefinitionType}
import overlord.Gateware.Gateware
import overlord.Software.Software

import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer
import ikuy_utils._

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
	def fromFile(name: String, spath: Path): Option[Seq[DefinitionTrait]] = {
		println(s"Reading $name catalog")

		val path = spath.resolve(s"${name}.toml")

		if (!Files.exists(path.toAbsolutePath)) {
			println(s"${name} catalog at ${path} not found");
			return None
		}

		val source = Utils.readToml(name, path, getClass)
		parse(name, source, spath)
	}

	private def parse(name: String,
	                  parsed: Map[String, Variant],
	                  spath: Path): Option[Seq[DefinitionTrait]] = {

		val path = if (parsed.contains("path")) {
			spath.resolve(Utils.toString(parsed("path")))
		} else spath

		var defs = ArrayBuffer[DefinitionTrait]()

		if (parsed.contains("definition")) {
			val chips = Utils.toArray(parsed("definition"))
			for (chip <- chips)
				defs += Definition(chip, path)
		}

		Some(defs.toSeq)
	}
}