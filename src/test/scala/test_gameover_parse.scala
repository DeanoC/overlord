import org.scalatest.flatspec.AnyFlatSpec
import scala.io.Source
import java.io.InputStream
import toml._;


class GameOverTest extends AnyFlatSpec {

  val stream: InputStream = getClass.getResourceAsStream("/game.over")
  val gameOver = scala.io.Source.fromInputStream( stream ).mkString

  "game.over" should "exist" in {
    assert(gameOver.nonEmpty)
  }
  "toml of game.over" should "parse ast" in {
    assert(toml.Toml.parse(gameOver).isRight)
  }
  "toml of game.over" should "has global key/value pairs that are okay" in {
    val table = toml.Toml.parse(gameOver)
    assert(table.isRight)
    val t = table.getOrElse(Value.Tbl(Map[String,Value]()))
    assert(t.values("board").asInstanceOf[toml.Value.Str].value == "Myir-FZ3")
    assert(t.values("bootcpu").asInstanceOf[toml.Value.Str].value == "SaxonSoC.riscv32")
    assert(t.values("layout").asInstanceOf[toml.Value.Arr].values.nonEmpty)
  }
/*
  "toml of game.over ram array" should "just have DDR4 entry" in {
    val table = toml.Toml.parse(gameOver)
    assert(table.isRight)
    val t = table.getOrElse(Value.Tbl(Map[String,Value]()))
    assert(t.values("ram").asInstanceOf[toml.Value.Arr].values.nonEmpty)
    val ram = t.values("ram").asInstanceOf[toml.Value.Arr]
    assert(ram.values.length == 1)
    val ddr4kv = ram.values(0).asInstanceOf[toml.Value.Tbl]
    println(s"${ram}")
    assert(ddr4kv.values("name").asInstanceOf[toml.Value.Str].value == "DDR4")
    assert(ddr4kv.values("size").asInstanceOf[toml.Value.Str].value == "4 GiB")
  }*/
}