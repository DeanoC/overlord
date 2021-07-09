package output

import ikuy_utils.Utils
import overlord.Game

import java.nio.file.Path
import java.util.Calendar

object BaseCHeaders {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating Base Headers at $out")
		Utils.ensureDirectories(out)

		genCpuHeaders(game, out)
	}
	private def genCpuHeaders(game: Game, out: Path): Unit = {
		val dT = Calendar.getInstance()

		for(cpu <- game.cpus) {
			val sb = new StringBuilder
			sb ++= f"#pragma once%n"
			sb ++= f"// Copyright Deano Calver%n"
			sb ++= f"// SPDX-License-Identifier: MIT%n"
			sb ++= f"// ${cpu.width} bit ${cpu.triple} CPU%n"
			sb ++= f"// Auto-generated on ${dT.getTime}%n"
			val connectedBuses = game.getBusesConnectedToCpu(cpu)
			sb ++= f"// ${connectedBuses.length} buses%n"

			val path = out.resolve(s"${cpu.sanitized_triple}")
			Utils.ensureDirectories(path)
			Utils.writeFile(path.resolve(s"platform.h"), sb.result())
		}
	}
}
