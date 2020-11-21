package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path

import overlord.{Game, GameBuilder}

object Project {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating project at ${out.toRealPath()}")

		val softPath = out.resolve("soft")
		val gatePath = out.resolve("gate")
		ensureDirectories(softPath)
		ensureDirectories(gatePath)

		GameBuilder.pathStack.push(gatePath.toRealPath())
		for {(gateware, defi) <- game.gatewares} {
			val backupStack = GameBuilder.pathStack.clone()

			for {action <- defi.actions} {
				val parameters = defi.parameters
				val merged     = game.constants
					.map(_.asParameter)
					.fold(parameters)((o, n) => o ++ n)
				action.execute(gateware, merged, GameBuilder.pathStack.top)
			}
			GameBuilder.pathStack = backupStack
		}

		output.Xdc(game, gatePath)
		output.Top(game, gatePath)
		output.Edalize(game, gatePath)
		if (game.cpus.nonEmpty)
			output.Svd(game, softPath)
	}

	def ensureDirectories(path: Path): Unit = {
		val directory = path.toFile
		if (!directory.exists()) {
			directory.mkdirs()
		}
	}

	private def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}

}
