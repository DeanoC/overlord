package output

import ikuy_utils.Utils
import overlord.Game

import java.nio.file.Path

object Software {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return
		println(s"Creating Software at $out")

		Utils.ensureDirectories(out)
		val hostProgramPath = out.resolve("programs_host")
		val libraryPath     = out.resolve("libraries")

		Utils.ensureDirectories(hostProgramPath)
		Utils.ensureDirectories(libraryPath)

		val in_program_paths       = game.cpus
			.map("programs_" + _.splitIdent.last)
			.map(tmp_program_path(out).resolve)
		val out_program_paths      = game.cpus
			.map("programs_" + _.splitIdent.last)
			.map(out.resolve)
		val programs_folder_exists = in_program_paths.map(Utils.doesFileOrDirectoryExist)
		for ((exists, i) <- programs_folder_exists.zipWithIndex) {
			if (exists) {
				Utils.deleteDirectories(out_program_paths(i))
				Utils.ensureDirectories(out_program_paths(i))
			}
		}

		relocateTmpSoftware(game, out)

		output.Compiler(game, hostProgramPath)
		//		output.Svd(game, out)
		genScripts(game, out, out_program_paths, programs_folder_exists)

	}

	def tmp_program_path(out: Path): Path = out.resolve("build")
		.resolve("tmp")
		.resolve("soft")

	private def relocateTmpSoftware(game: Game, out: Path): Unit = {
		Utils.deleteDirectories(out.resolve("libraries"))

		// move from the tmp folder we done some building to its real place
		Utils.rename(out.resolve("build")
			             .resolve("tmp")
			             .resolve("soft")
			             .resolve("libraries"),
		             out.resolve("libraries"))
		Utils.copy(Path.of("software/subdir.cmake"),
		           out.resolve("libraries").resolve("CMakeLists.txt"),
		           getClass)
		//		Utils.rename(out.resolve("build")
		//			             .resolve("tmp")
		//			             .resolve("soft")
		//			             .resolve("programs_host"),
		//		             out.resolve("programs_host"))

		for (cpu <- game.cpus) {
			val targetPrograms = s"programs_${cpu.splitIdent.last}"
			Utils.deleteDirectories(out.resolve(targetPrograms))

			Utils.rename(out.resolve("build")
				             .resolve("tmp")
				             .resolve("soft")
				             .resolve(targetPrograms),
			             out.resolve(targetPrograms))
		}
	}

	private def genScripts(game: Game,
	                       out: Path,
	                       program_paths: Seq[Path],
	                       program_path_exists: Seq[Boolean]): Unit = {
		val sb = new StringBuilder
		for ((cpu, i) <- game.cpus.zipWithIndex) {
			if (program_path_exists(i)) {
				val sbpm = new StringBuilder
				val sbm  = new StringBuilder

				val cpu_name     = cpu.splitIdent.last
				val program_name = "programs_" + cpu_name

				Utils.rename(program_paths(i), out.resolve(program_name))

				sbpm ++=
				f"cmake -DCMAKE_TOOLCHAIN_FILE=$$PWD/${cpu_name}_toolchain.cmake " +
				f"-G Ninja -S . -B build/$program_name -DCPU=$cpu_name%n"

				sbm ++= f"cmake --build build/$program_name%n"

				sb ++= f"./premake_$program_name.sh%n"
				sb ++= f"./make_$program_name.sh%n"

				val premakePath = out.resolve(s"premake_$program_name.sh")
				val makePath    = out.resolve(s"make_$program_name.sh")
				Utils.writeFile(premakePath, sbpm.result())
				Utils.setFileExecutable(premakePath)
				Utils.writeFile(makePath, sbm.result())
				Utils.setFileExecutable(makePath)

				Utils.copy(Path.of("software/subdir.cmake"),
				           out.resolve(s"$program_name").resolve("CMakeLists.txt"),
				           getClass)
			}
		}
		Utils.writeFile(out.resolve(s"make_programs.sh"), sb.result())
		Utils.setFileExecutable(out.resolve(s"make_programs.sh"))
		Utils.copy(Path.of("software/ikuy_root.cmake"),
		           out.resolve("CMakeLists.txt"),
		           getClass)
		// workaround for microblaze gcc
		Utils.copy(Path.of("software/empty-file.ld"),
		           out.resolve("empty-file.ld"),
		           getClass)

		/*
		for(prgs <- game.programs) {
			val cpus = if (prgs.attributes.contains("cpus")) {
				Utils.toArray(prgs.attributes("cpus")).map(Utils.toString)
			} else Array[String]()

			for (cpu <- game.cpus) {
				val cpuName = cpu.splitIdent.last

				val forThisCpu = if(cpus.nonEmpty)
					cpus.contains(cpuName)
				else true

				if(forThisCpu) {
					val sanTriple = cpu.sanitizedTriple
					sb ++=
					f"cmake -D CMAKE_TOOLCHAIN_FILE=$$PWD/${cpuName}_toolchain.cmake " +
					f"-G Ninja " +
					f"-S programs_${cpu.splitIdent.last} " +
					f"-B build/programs_${cpu.splitIdent.last} " +
					"\n"

					sb ++= f"cmake --build build/programs_${cpu.splitIdent.last}%n"
				}
			}
		}*/

		Utils.writeFile(out.resolve("make_software.sh"), sb.result())
	}

}
