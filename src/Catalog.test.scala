//> using lib "com.lihaoyi::utest::0.7.10"
package Overlord

import utest._

object CatalogTests extends TestSuite {
  val targetPath = os.temp.dir() / "CatalogTest"

  // if exist, remove
  val paths = if !os.exists(targetPath) then
    // create directories
    os.makeDir.all(targetPath)
    assert(os.exists(targetPath))
    val tempPath = targetPath / "tmp"
    os.makeDir.all(tempPath)
    val binPath = targetPath / "bin"
    os.makeDir.all(binPath)
    Paths(targetPath, tempPath, binPath)
  else Paths(targetPath, targetPath, targetPath)

  gitClone(paths, "git@github.com:DeanoC/ikuy_std_resources.git", "ikuy_std_resources")
  val definitions = Catalog(paths.targetPath / "ikuy_std_resources" / "definitions")

  val tests = Tests {

    test("check boards") {
      definitions.matchIdentifier(Identifier(Seq("boards")))
    }
    test("check boards.host") {
      definitions.matchIdentifier(Identifier(Seq("boards", "host")))
    }
  }
}
