import java.io.InputStream

import org.scalatest.flatspec.AnyFlatSpec


class MyirOverTest extends AnyFlatSpec {

	val stream: InputStream = getClass.getResourceAsStream("/myir.over")
	val gameOver = scala.io.Source.fromInputStream(stream).mkString

	"game.over" should "exist" in {
		assert(gameOver.nonEmpty)
	}
	"toml of myir.over" should "parse ast" in {
		assert(toml.Toml.parse(gameOver).isRight)
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