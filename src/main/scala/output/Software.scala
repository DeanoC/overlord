package output

import ikuy_utils.Utils
import overlord.Instances.LibraryInstance
import overlord.{Game, Resources}

import java.nio.file.Path

object Software {
	private val cpuRegEx = "\\s*,\\s*".r

	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return
		println(s"Creating Software at $out")

		Utils.ensureDirectories(out)
		Utils.ensureDirectories(out.resolve("programs_host"))
		Utils.ensureDirectories(out.resolve("libs"))

		output.Compiler(game, out)

		val program_paths          = game.cpus
			.map("programs_" + _.definition.defType.ident.last)
		val in_program_paths       = program_paths.map(tmp_program_path(out).resolve)
		val out_program_paths      = program_paths.map(out.resolve)
		val programs_folder_exists = in_program_paths.map(Utils.doesFileOrDirectoryExist)
		game.cpus
			.foreach(cpu => Utils.ensureDirectories(out.resolve(
				"programs_" + cpu.definition.defType.ident.last)))

		relocateTmpSoftware(game, out)

		//		output.Svd(game, out)
		genScripts(game, out, out_program_paths, programs_folder_exists)

	}

	def tmp_program_path(out: Path): Path = out.resolve("build")
		.resolve("tmp")
		.resolve("soft")

	private def relocateTmpSoftware(game: Game, out: Path): Unit = {
		Utils.deleteDirectories(out.resolve("libs"))

		// move from the tmp folder we done some building to its real place
		Utils.rename(out.resolve("build")
			             .resolve("tmp")
			             .resolve("soft")
			             .resolve("libs"),
		             out.resolve("libs"))

		generateSubdirMake(out.resolve("libs").resolve("CMakeLists.txt"))

		val libraries = game.allSoftwareInstances.flatMap { l =>
			l match {
				case instance: LibraryInstance => Some(instance)
				case _                         => None
			}
		}

		val tmpPath = out.resolve("build").resolve("tmp").resolve("soft")
		val libPath = out.resolve("libs")

		for (cpu <- game.cpus) {
			val cpuName        = cpu.definition.defType.ident.last
			// replace programs we might have change but leave existing along (i.e. compilers)
			val targetPrograms = s"programs_$cpuName"
			val sourcePath     = tmpPath.resolve(targetPrograms)
			val targetPath     = out.resolve(targetPrograms)
			val df             = sourcePath.toFile
			if (df.exists()) {
				df.listFiles.foreach(f => {
					val alreadyExists = Utils.doesFileOrDirectoryExist(targetPath.resolve(f
						                                                                      .getName))
					if (alreadyExists) {
						Utils.deleteDirectories(targetPath.resolve(f.getName))
					}
					Utils.rename(f.toPath, targetPath.resolve(f.getName))
				})
			}
			// create per platform library symbolic links
			val cpuLibPath = out.resolve(s"libs_$cpuName")
			Utils.deleteDirectories(cpuLibPath)
			Utils.ensureDirectories(cpuLibPath)
			Utils.copy(Resources.stdResourcePath().resolve("catalogs/software/subdir.cmake"),
			           out.resolve(s"libs_$cpuName").resolve("CMakeLists.txt"),
			           getClass)

			for (lib <- libraries) {
				val all_cpus = if (lib.attributes.contains("cpus")) {
					val cpusString = Utils.toString(lib.attributes("cpus"))
					if (cpusString == "_") None
					else Some(cpuRegEx.split(cpusString).toSeq)
				} else None

				if (all_cpus.isEmpty || all_cpus.get.contains(cpuName)) {
					val sourcePath = libPath.resolve(lib.name)
					val targetPath = out.resolve(s"libs_$cpuName").resolve((lib.name))

					Utils.createSymbolicLink(sourcePath, targetPath)
				}
			}

		}
	}

	private def generateSubdirMake(out: Path, excludes: Seq[String] = Seq()): Unit = {
		val sb = new StringBuilder
		sb ++=
		"""file(GLOB _all LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			|CONFIGURE_DEPENDS * )
			|list(REMOVE_ITEM _all "CMakeLists.txt" )
			|""".stripMargin

		for (ed <- excludes) {
			sb ++=
			f"""list(REMOVE_ITEM _all "${ed}")
				 |""".stripMargin
		}

		sb ++=
		"""
			|foreach(sd ${_all})
			|   add_subdirectory(${sd})
			|endforeach()""".stripMargin

		Utils.writeFile(out, sb.result())
	}

	private def genScripts(game: Game,
	                       out: Path,
	                       programPaths: Seq[Path],
	                       programPathExists: Seq[Boolean]): Unit = {
		val sb = new StringBuilder
		for ((cpu, i) <- game.cpus.zipWithIndex) {
			if (programPathExists(i)) {
				val sbpm = new StringBuilder
				val sbm  = new StringBuilder

				val cpuName     = cpu.definition.defType.ident.last
				val programName = "programs_" + cpuName

				Utils.rename(programPaths(i), out.resolve(programName))

				sbpm ++=
				f"cmake -DCMAKE_TOOLCHAIN_FILE=$$PWD/${cpuName}_toolchain.cmake " +
				f"-G Ninja -S . -B build/$programName -DCPU=$cpuName -DBOARD=${
					game
						.board
						.get
						.ident
				}%n"

				sbm ++= f"cmake --build build/$programName%n"

				sb ++= f"./premake_$programName.sh%n"
				sb ++= f"./make_$programName.sh%n"

				val premakePath = out.resolve(s"premake_$programName.sh")
				val makePath    = out.resolve(s"make_$programName.sh")
				Utils.writeFile(premakePath, sbpm.result())
				Utils.setFileExecutable(premakePath)
				Utils.writeFile(makePath, sbm.result())
				Utils.setFileExecutable(makePath)

				val excludes = if (cpuName == "host") {
					game.cpus
						.flatMap { cpu => if (cpu.definition.defType.ident.last ==
						                      "host") None else Some(cpu.triple)
						}
				} else Seq()

				generateSubdirMake(out.resolve(s"$programName").resolve("CMakeLists.txt"),
				                   excludes)
			}
		}
		Utils.writeFile(out.resolve(s"make_programs.sh"), sb.result())
		Utils.setFileExecutable(out.resolve(s"make_programs.sh"))
		Utils.copy(Resources.stdResourcePath().resolve("catalogs/software/ikuy_root.cmake"),
		           out.resolve("CMakeLists.txt"),
		           getClass)
		// workaround for microblaze gcc
		Utils.copy(Resources.stdResourcePath().resolve("catalogs/software/empty-file.ld"),
		           out.resolve("empty-file.ld"),
		           getClass)

		Utils.writeFile(out.resolve("make_software.sh"), sb.result())
	}

}
