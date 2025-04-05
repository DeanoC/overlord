package overlord

import java.nio.file.{Files, Path, Paths}
import ikuy_utils.Utils

/**
 * Resources class for handling resource loading in the Scala 3 version
 */
class Resources(path: Path):
  def loadCatalogs(): Map[String, Any] = 
    println(s"Loading catalogs from $path")
    Map.empty // Stub implementation
    
  def loadPrefabs(): Map[String, Any] =
    println(s"Loading prefabs from $path")
    Map.empty // Stub implementation

object Resources:
  def apply(path: Path): Resources = new Resources(path)
  
  def stdResourcePath(): Path =
    // In a real implementation, this would look up the standard resource path
    Paths.get(System.getProperty("user.dir"), "resources")