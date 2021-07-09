package output

import java.nio.file.Path

import overlord.Game

object Project {
	def apply(game: Game, out: Path): Unit = {
		println(s"Creating project at ${out.toRealPath()}")

		val softPath = out.resolve("soft")
		val gatePath = out.resolve("gate")

		output.Report(game, gatePath)
		output.Xdc(game, gatePath)
		output.Top(game, gatePath)
		output.Edalize(game, gatePath)

		if (game.cpus.nonEmpty) {
			output.Compiler(game, softPath)
			output.BaseCHeaders(game, softPath.resolve("include"))
			output.BootRom(game, softPath.resolve("bootroms"))
			output.Svd(game, softPath)
		}
	}
}

object UpdateProject {
	def apply(game: Game, out: Path, instance: Option[String]): Unit = {
		instance match {
			case Some(inst) =>

			case None        =>
				// TODO for now just call create Project
				Project(game, out)
		}
	}

}
