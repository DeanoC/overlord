//> using lib "com.lihaoyi::utest::0.7.10"
package Overlord

import utest._

object CatalogTests extends TestSuite {
  val targetPath = os.temp.dir() / "CatalogTest"

  val paths = Paths(targetPath, targetPath / "tmp", targetPath / "bin", targetPath / "libs")

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
