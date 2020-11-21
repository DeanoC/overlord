package output

import java.io.{BufferedWriter, FileWriter}
import java.nio.file.Path

import overlord.{ClockPinConstraint, DiffPinConstraint, Game, PinConstraint,
	Utils}

object Xdc {
	def apply(game: Game, out: Path): Unit = {
		val sb = new StringBuilder

		val conCon = game.connectedConstraints.toSet
		for (con <- conCon) {
			con.constraintType match {
				case PinConstraint(pins)     =>
					val standard = Utils.toString(con.attributes("standard"))
					if (pins.length > 1) {
						for ((p, i) <- pins.zipWithIndex)
							sb ++=
							s"""set_property -dict {
								 |    PACKAGE_PIN ${p}
								 |    IOSTANDARD ${standard}
								 |} [get_ports {${sanatizeIdent(con.ident)}[$i]}];
								 |""".stripMargin
					} else {
						sb ++=
						s"""set_property -dict {
							 |    PACKAGE_PIN ${pins.head}
							 |    IOSTANDARD ${standard}
							 |} [get_ports {${sanatizeIdent(con.ident)}}];
							 |""".stripMargin

					}
				case DiffPinConstraint(pins) =>
				case ClockPinConstraint()    =>
			}

		}

		writeFile(out.resolve(s"${game.name}.xdc"), sb.result())
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
