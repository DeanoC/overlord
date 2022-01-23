package overlord

import ikuy_utils.{Utils, Variant}

import java.nio.file.{Files, Path}
import scala.collection.mutable

case class Prefab(name: String, instances: Map[String, Variant])

class PrefabCatalog {
	type key = String
	type value = Prefab
	type keyStore = mutable.HashMap[key, value]

	val prefabs: keyStore = mutable.HashMap()

	def findPrefab(name: key): Option[value] = {
		if (prefabs.contains(name)) Some(prefabs(name))
		else None
	}

}

object PrefabCatalog {
	def fromFile(filename: String,
	             spath: Path): Seq[Prefab] = {
		println(s"Reading $filename prefab")

		val path = spath.resolve(s"$filename.toml")

		if (!Files.exists(path.toAbsolutePath)) {
			println(s"$filename catalog at $path not found")
			return Seq()
		}
		val source  = Utils.readToml(filename, path, getClass)
		var prefabs = Array[Prefab]()

		if (source.contains("resources")) {
			val resources = Utils.lookupArray(source, "resources")
			for (resource <- resources) {
				val rname = Utils.toString(resource)
				prefabs ++= PrefabCatalog.fromFile(s"$rname", spath)
			}
		}
		val name  = filename.replace("/", ".")
		val stuff = source.filter { s =>
			s._1 == "instance" ||
			s._1 == "connection" ||
			s._1 == "path" ||
			s._1 == "prefab" ||
			s._1 == "include"
		}
		if (stuff.nonEmpty) {
			prefabs ++= Seq(Prefab(name, stuff))
		}
		prefabs.toSeq
	}
}