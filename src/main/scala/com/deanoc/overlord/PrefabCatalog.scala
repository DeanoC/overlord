package com.deanoc.overlord

import com.deanoc.overlord.utils.{Logging, ArrayV, Utils, Variant}
import com.deanoc.overlord.Overlord

import java.nio.file.Files
import scala.collection.mutable
import scala.collection.immutable
import scala.util.boundary, boundary.break

// Represents a Prefab with its name, path, associated data (stuff), and included prefabs.
case class Prefab(
    name: String,
    path: String,
    stuff: Map[String, Variant],
    includes: Map[String, Prefab]
)

// Documentation: How to Create a Prefab
// A Prefab represents a reusable component with a name, path, associated data ("stuff"), and included prefabs. To create a prefab:
// 1. Define a unique name for the prefab.
// 2. Specify the path where the prefab is located.
// 3. Provide a map of "stuff" containing key-value pairs, where values are of type Variant.
//    - Common keys include "instance" and "connection".
// 4. Optionally, include other prefabs by providing a map of included prefab names to Prefab objects.
// 5. Use the PrefabCatalog class to manage and retrieve prefabs.

class PrefabCatalog(
    val prefabs: PrefabCatalog#KeyStore = immutable.HashMap.empty
) extends Logging {
  type KeyStore = immutable.HashMap[String, Prefab]

  // Finds a prefab by its name.
  def findPrefab(name: String): Option[Prefab] = {
    // Check if the prefab exists in the catalog.
    if (prefabs.contains(name)) Some(prefabs(name))
    else None
  }
}

object PrefabCatalog extends Logging {

  // Reads prefabs from a file and returns a catalog, will produce a flattened catalog
  // from any includes found in the file.
  def fromFile(
      fileName: String,
      existing: PrefabCatalog = PrefabCatalog()
  ): PrefabCatalog = {
    val filePath = Overlord.instancePath.resolve(fileName)

    if (!Files.exists(filePath.toAbsolutePath)) {
      info(s"$fileName catalog at $filePath not found")
      existing
    } else if (Files.size(filePath.toAbsolutePath) == 0) {
      info(s"$fileName is empty creating an empty prefab")
      existing
    } else {
      // Push the file's parent path to the instance path stack.
      Overlord.pushInstancePath(filePath.getParent)
      val source = Utils.readYaml(filePath)

      // Handle invalid or comment-only files by creating an empty prefab.
      if (source == null) {
        info(s"$fileName is comments only or invalid, creating an empty prefab")
        Overlord.popInstancePath()
        existing
      } else {

        var newPrefabs = mutable.HashMap(existing.prefabs.toSeq: _*)

        // Includes are lists of prefab catalogs to include.
        // This is a recursive process, so we need to keep track of the current
        // prefab catalog and merge it with the new one.
        if (source.contains("includes")) {
          val includes = Utils.lookupArray(source, "includes")
          var prefabCatalog =
            for (prefab <- includes)
              yield PrefabCatalog.fromFile(Utils.toString(prefab))

          newPrefabs = prefabCatalog.foldLeft(newPrefabs)((acc, catalog) =>
            acc ++ catalog.prefabs
          )
        }

        // now process the actual prefabs in the file
        var prefabs = Array[Prefab]()
        val name = fileName.replace("/", ".")
        val included = mutable.HashMap[String, Prefab]()

        // Extract relevant data from the source create the prefabs.
        val stuff = source.filter { s =>
          s._1 == "instance" ||
          s._1 == "connection" ||
          s._1 == "prefab" ||
          s._1 == "includes"
        }
        if (stuff.nonEmpty)
          prefabs ++= Seq(
            Prefab(name, Overlord.instancePath.toString, stuff, included.toMap)
          )

        // Handle cases where no usable data is found.
        if (prefabs.isEmpty) {
          warn(
            s"$fileName contains no usable data, creating an empty prefab"
          )
          val name = fileName.replace("/", ".")
          prefabs ++= Seq(
            Prefab(name, Overlord.instancePath.toString, Map.empty, Map.empty)
          )
        }

        // Pop the instance path stack.
        Overlord.popInstancePath()

        PrefabCatalog(immutable.HashMap(newPrefabs.toSeq: _*))
      }
    }
  }
}
