package overlord

import org.scalatest.funsuite.AnyFunSuite
import java.nio.file.{Path, Paths}

class ResourcesSpec extends AnyFunSuite {

  test("stdResourcePath should return an absolute path") {
    val path = Resources.stdResourcePath()
    assert(path.isAbsolute, "The standard resource path should be an absolute path")
  }

  test("stdResourcePath should contain 'gagameos_stdcatalog' in the path") {
    val path = Resources.stdResourcePath()
    assert(path.toString.contains("gagameos_stdcatalog"), 
      s"The path should contain 'gagameos_stdcatalog', but was: ${path.toString}")
  }

  test("stdResourcePath should correctly handle tilde expansion") {
    val path = Resources.stdResourcePath()
    val userHome = System.getProperty("user.home")
    
    // Ensure the path does not contain a tilde and is correctly expanded
    assert(!path.toString.contains("~"), "The path should not contain a tilde (~)")
    assert(path.toString.startsWith(userHome), 
      s"The path should start with the user's home directory ($userHome), but was: ${path.toString}")
  }
}