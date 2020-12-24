package output

import java.nio.file.Path

import overlord.Game

object Project {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating project at ${out.toRealPath()}")

		val softPath = out.resolve("soft")
		val gatePath = out.resolve("gate")

		output.Xdc(game, gatePath)
		output.Top(game, gatePath)
		output.Edalize(game, gatePath)

		if (game.cpus.nonEmpty) {
			output.Compiler(game, softPath)
			output.Svd(game, softPath)
		}
	}
}
