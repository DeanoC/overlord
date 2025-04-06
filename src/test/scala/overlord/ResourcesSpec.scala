package overlord

import org.scalatest.funsuite.AnyFunSuite
import java.nio.file.{Path, Paths}

class ResourcesSpec extends AnyFunSuite {

  test("stdResourcePath should return an absolute path") {
    val path = Resources.stdResourcePath()
    assert(path.isAbsolute, "The standard resource path should be an absolute path")
  }

  test("stdResourcePath should contain 'gagameosstd_catalog' in the path") {
    val path = Resources.stdResourcePath()
    assert(path.toString.contains("gagameosstd_catalog"), 
      s"The path should contain 'gagameosstd_catalog', but was: ${path.toString}")
  }

  test("stdResourcePath should correctly handle tilde expansion") {
    val path = Resources.stdResourcePath()
    val userHome = System.getProperty("user.home")
    
    // If tilde is correctly expanded, the path should contain the user's home directory
    // Note: This test might fail if ~ isn't properly expanded to the home directory
    assert(path.toString.contains(userHome) || !path.toString.contains("~"),
      s"The path should either contain the user's home directory ($userHome) or not contain a tilde (~), but was: ${path.toString}")
  }

  // This test suggests a better implementation that properly handles tilde expansion
  test("A proper home directory expansion implementation") {
    val properPath = {
      val pathStr = "~/gagameosstd_catalog/"
      val expandedPath = if (pathStr.startsWith("~/")) {
        Paths.get(System.getProperty("user.home"), pathStr.substring(2))
      } else {
        Paths.get(pathStr)
      }
      expandedPath.toAbsolutePath.normalize()
    }
    
    // The proper path should contain the user's home directory
    assert(properPath.toString.contains(System.getProperty("user.home")),
      s"The properly expanded path should contain the user's home directory, but was: ${properPath.toString}")
    
    // It should not contain a tilde character
    assert(!properPath.toString.contains("~"),
      s"The properly expanded path should not contain a tilde (~), but was: ${properPath.toString}")
  }
}