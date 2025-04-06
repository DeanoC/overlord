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

  test("Option parsing should correctly handle -y flag") {
    val args = Array("create", "-y", "--board", "test-board", "example.over")
    val options = MainUtils.nextOption(Map(), args.toList)

    assert(options(Symbol("yes")) == true)
    assert(options(Symbol("board")) == "test-board")
    assert(options(Symbol("infile")) == "example.over")
  }

  test("Option parsing should correctly handle --yes flag") {
    val args = Array("create", "--yes", "--board", "test-board", "example.over")
    val options = MainUtils.nextOption(Map(), args.toList)

    assert(options(Symbol("yes")) == true)
    assert(options(Symbol("board")) == "test-board")
    assert(options(Symbol("infile")) == "example.over")
  }

  test("Option parsing should handle both resource flags and auto download options") {
    val args = Array(
      "create", 
      "--nostdresources", 
      "--resources", "./custom-resources", 
      "--yes", 
      "--board", "test-board", 
      "example.over"
    )
    val options = MainUtils.nextOption(Map(), args.toList)

    assert(options(Symbol("nostdresources")) == true)
    assert(options(Symbol("resources")) == "./custom-resources")
    assert(options(Symbol("yes")) == true)
    assert(options(Symbol("board")) == "test-board")
    assert(options(Symbol("infile")) == "example.over")
  }

  test("Non-interactive console should set 'yes' option to true") {
    val args = Array("create", "--board", "test-board", "example.over")
    val options = MainUtils.nextOption(Map(), args.toList).updated(Symbol("yes"), true)

    assert(options(Symbol("yes")) == true)
    assert(options(Symbol("board")) == "test-board")
    assert(options(Symbol("infile")) == "example.over")
  }
}