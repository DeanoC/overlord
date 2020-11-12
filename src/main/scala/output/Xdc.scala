package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path

import output.Edalize.writeFile
import overlord.{ClockPinConstraint, DiffPinConstraint, Game, PinConstraint}

object Xdc {
	def apply(game: Game, out : Path): Unit = {
		val sb = new StringBuilder

		for(ci <- game.connectedConstraints) {
			val con = ci.constraint
			con.constraintType match {
				case PinConstraint(pins)     =>
					sb ++=
					s"""set_property -dict {
						 |    PACKAGE_PIN ${pins.head}
						 |    IOSTANDARD ${con.attributes("standard").asInstanceOf[toml.Value.Str].value}
						 |} [get_ports {${sanatizeIdent(con.ident)}}];
						 |""".stripMargin
				case DiffPinConstraint(pins) =>
				case ClockPinConstraint()    =>
			}

		}

		writeFile(out.resolve(s"${game.name}.xdc"), sb.result)
	}

	private def ensureDirectories(path: Path): Unit = {
		val directory = path.toFile
		if (directory.isDirectory && !directory.exists()) {
			directory.mkdirs()
		}
	}

	private def sanatizeIdent(in: String): String = {
		in
			.replaceAll("""->|\.""", "_")
	}

	private def writeFile(path: Path, s: String): Unit = {
		val file = path.toFile
		val bw   = new BufferedWriter(new FileWriter(file))
		bw.write(s)
		bw.close()
	}
}
