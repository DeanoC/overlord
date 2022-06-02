package overlord

import ikuy_utils.{ArrayV, Utils, Variant}

import java.nio.file.Files
import scala.collection.mutable

case class Prefab(name: String,
                  path: String,
                  stuff: Map[String, Variant],
                  includes: Map[String, Prefab]
                 )

class PrefabCatalog {
	type KeyStore = mutable.HashMap[String, Prefab]

	val prefabs: KeyStore = mutable.HashMap()

	def findPrefab(name: String): Option[Prefab] = {
		if (prefabs.contains(name)) Some(prefabs(name))
		else None
	}

	def findPrefabInclude(include: String): Option[Prefab] = {
		prefabs.foreach { case (_, prefab) => if (prefab.includes.contains(include)) return Some(prefab.includes(include)) }
		None
	}

	def flattenIncludesContents(include: String): Map[String, Variant] = {
		require(findPrefabInclude(include).nonEmpty)
		val startPrefab = findPrefabInclude(include).get

		val gatherInstances   = mutable.ArrayBuffer[Variant]()
		val gatherConnections = mutable.ArrayBuffer[Variant]()
		flattenIncludesContentsInternal(startPrefab, gatherInstances, gatherConnections)
		val gather = mutable.HashMap[String, Variant]("instance" -> ArrayV(gatherInstances.toArray), "connection" -> ArrayV(gatherConnections.toArray))
		gather.toMap
	}

	private def flattenIncludesContentsInternal(prefab: Prefab,
	                                            gatherInstances: mutable.ArrayBuffer[Variant],
	                                            gatherConnections: mutable.ArrayBuffer[Variant]
	                                           ): Unit = {
		if (prefab.stuff.contains("instance")) gatherInstances ++= prefab.stuff("instance").asInstanceOf[ArrayV].value
		if (prefab.stuff.contains("connection")) gatherConnections ++= prefab.stuff("connection").asInstanceOf[ArrayV].value
		prefab.includes.foreach(p => flattenIncludesContentsInternal(p._2, gatherInstances, gatherConnections))

	}
}

object PrefabCatalog {
	def fromFile(fileName: String): Seq[Prefab] = {
		val filePath = Game.instancePath.resolve(fileName)

		println(s"Reading $fileName prefab")

		if (!Files.exists(filePath.toAbsolutePath)) {
			println(s"$fileName catalog at $filePath not found")
			return Seq()
		}

		Game.pushInstancePath(filePath.getParent)
		val source   = Utils.readToml(filePath)
		var prefabs  = Array[Prefab]()
		val includes = mutable.HashMap[String, Prefab]()

		if (source.contains("resources")) {
			val resources = Utils.lookupArray(source, "resources")
			for (resource <- resources) {
				val incResourceName = Utils.toString(resource)
				val includePayload  = PrefabCatalog.fromFile(s"$incResourceName")
				prefabs ++= includePayload
			}
		}

		// includes
		if (source.contains("include")) {
			val tincs = Utils.toArray(source("include"))
			for (include <- tincs) {
				val table           = Utils.toTable(include)
				val incResourceName = Utils.toString(table("resource"))
				val includePayload  = PrefabCatalog.fromFile(s"$incResourceName")
				includePayload.foreach(i => includes += (i.name -> i))
				prefabs ++= includePayload
			}
		}

		val name  = fileName.replace("/", ".")
		val stuff = source.filter { s =>
			s._1 == "instance" ||
			s._1 == "connection" ||
			s._1 == "prefab" ||
			s._1 == "include"
		}
		if (stuff.nonEmpty) prefabs ++= Seq(Prefab(name, Game.instancePath.toString, stuff, includes.toMap))

		Game.popInstancePath()
		prefabs.toSeq
	}
}