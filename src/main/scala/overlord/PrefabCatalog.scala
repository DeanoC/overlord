package overlord

import gagameos.{ArrayV, Utils, Variant}
import overlord.Project

import java.nio.file.Files
import scala.collection.mutable
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

class PrefabCatalog {
  type KeyStore = mutable.HashMap[String, Prefab]

  // Stores all prefabs in a mutable HashMap.
  val prefabs: KeyStore = mutable.HashMap()

  // Finds a prefab by its name.
  def findPrefab(name: String): Option[Prefab] = {
    // Check if the prefab exists in the catalog.
    if (prefabs.contains(name)) Some(prefabs(name))
    else None
  }

  // Finds a prefab include by its name.
  def findPrefabInclude(include: String): Option[Prefab] = {
    boundary {
      // Iterate through all prefabs and check if the include exists.
      prefabs.foreach { case (_, prefab) =>
        if (prefab.includes.contains(include))
          break(Some(prefab.includes(include)))
      }
    }
    None
  }

  // Flattens the contents of a prefab include into a map of instances and connections.
  def flattenIncludesContents(include: String): Map[String, Variant] = {
    require(findPrefabInclude(include).nonEmpty) // Ensure the include exists.
    val startPrefab = findPrefabInclude(include).get

    // Buffers to gather instances and connections.
    val gatherInstances = mutable.ArrayBuffer[Variant]()
    val gatherConnections = mutable.ArrayBuffer[Variant]()
    flattenIncludesContentsInternal(
      startPrefab,
      gatherInstances,
      gatherConnections
    )

    // Combine gathered data into a map.
    val gather = mutable.HashMap[String, Variant](
      "instance" -> ArrayV(gatherInstances.toArray),
      "connection" -> ArrayV(gatherConnections.toArray)
    )
    gather.toMap
  }

  // Recursively flattens the contents of a prefab and its includes.
  private def flattenIncludesContentsInternal(
      prefab: Prefab,
      gatherInstances: mutable.ArrayBuffer[Variant],
      gatherConnections: mutable.ArrayBuffer[Variant]
  ): Unit = {
    // Add instances and connections from the current prefab.
    if (prefab.stuff.contains("instance"))
      gatherInstances ++= prefab.stuff("instance").asInstanceOf[ArrayV].value
    if (prefab.stuff.contains("connection"))
      gatherConnections ++= prefab
        .stuff("connection")
        .asInstanceOf[ArrayV]
        .value

    // Recursively process included prefabs.
    prefab.includes.foreach(p =>
      flattenIncludesContentsInternal(p._2, gatherInstances, gatherConnections)
    )
  }
}

object PrefabCatalog {
  // Reads prefabs from a file and returns a sequence of Prefab objects.
  def fromFile(fileName: String): Seq[Prefab] = {
    val filePath = Project.instancePath.resolve(fileName)

    // Check if the file exists.
    if (!Files.exists(filePath.toAbsolutePath)) {
      println(s"$fileName catalog at $filePath not found")
      return Seq()
    }

    // Handle empty files by creating an empty prefab.
    if (Files.size(filePath.toAbsolutePath) == 0) {
      println(s"$fileName is empty creating an empty prefab")
      val name = fileName.replace("/", ".")
      return Seq(
        Prefab(name, Project.instancePath.toString, Map.empty, Map.empty)
      )
    }

    // Push the file's parent path to the instance path stack.
    Project.pushInstancePath(filePath.getParent)
    val source = Utils.readYaml(filePath)

    // Handle invalid or comment-only files by creating an empty prefab.
    if (source == null) {
      println(
        s"$fileName contains only comments or is invalid, creating an empty prefab"
      )
      val name = fileName.replace("/", ".")
      Project.popInstancePath()
      return Seq(
        Prefab(name, Project.instancePath.toString, Map.empty, Map.empty)
      )
    }

    var prefabs = Array[Prefab]()
    val includes = mutable.HashMap[String, Prefab]()

    // Process resources if present in the source.
    if (source.contains("resources")) {
      val resources = Utils.lookupArray(source, "resources")
      for (resource <- resources) {
        val incResourceName = Utils.toString(resource)
        val includePayload = PrefabCatalog.fromFile(s"$incResourceName")
        prefabs ++= includePayload
      }
    }

    // Process includes if present in the source.
    if (source.contains("include")) {
      val tincs = Utils.toArray(source("include"))
      for (include <- tincs) {
        val table = Utils.toTable(include)
        val incResourceName = Utils.toString(table("resource"))
        val includePayload = PrefabCatalog.fromFile(s"$incResourceName")
        includePayload.foreach(i => includes += (i.name -> i))
        prefabs ++= includePayload
      }
    }

    // Extract relevant data from the source to create a prefab.
    val name = fileName.replace("/", ".")
    val stuff = source.filter { s =>
      s._1 == "instance" ||
      s._1 == "connection" ||
      s._1 == "prefab" ||
      s._1 == "include"
    }
    if (stuff.nonEmpty)
      prefabs ++= Seq(
        Prefab(name, Project.instancePath.toString, stuff, includes.toMap)
      )

    // Handle cases where no usable data is found.
    if (prefabs.isEmpty) {
      println(s"$fileName contains no usable data, creating an empty prefab")
      val name = fileName.replace("/", ".")
      prefabs ++= Seq(
        Prefab(name, Project.instancePath.toString, Map.empty, Map.empty)
      )
    }

    // Pop the instance path stack.
    Project.popInstancePath()
    prefabs.toSeq
  }
}
