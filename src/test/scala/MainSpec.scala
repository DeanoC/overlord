import org.scalatest.funsuite.AnyFunSuite

class MainSpec extends AnyFunSuite {

  test("Option parsing should correctly parse create command with output path") {
    val args = Array("create", "--out", "./output", "--board", "test-board", "example.over")
    val options = MainUtils.nextOption(Map(), args.toList)

    assert(options(Symbol("create")) == true)
    assert(options(Symbol("out")) == "./output")
    assert(options(Symbol("board")) == "test-board")
    assert(options(Symbol("infile")) == "example.over")
  }

  test("Option parsing should fail when no infile is provided") {
    val args = Array("create", "--out", "./output")

    intercept[Exception] {
      MainUtils.nextOption(Map(), args.toList)
    }
  }

  test("Option parsing should fail when no board is provided") {
    val args = Array("create", "example.over")

    intercept[Exception] {
      MainUtils.nextOption(Map(), args.toList)
    }
  }
}