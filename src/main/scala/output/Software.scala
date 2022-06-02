package output

import ikuy_utils.Utils
import overlord.Instances.LibraryInstance
import overlord.{Game, Resources}

import java.nio.file.Path
import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer

object Software {
	private val cpuRegEx = "\\s*,\\s*".r

	def apply(game: Game): Unit = {
		if (game.cpus.isEmpty) return
		println(s"Creating Software at ${Game.outPath}")

		val out = Game.outPath

		Utils.ensureDirectories(out)
		Utils.ensureDirectories(out.resolve("programs_host"))
		Utils.ensureDirectories(out.resolve("libs"))

		output.Compiler(game, out)

		val program_paths          = game.cpus.map("programs_" + _.cpuType)
		val in_program_paths       = program_paths.map(tmp_program_path(out).resolve)
		val out_program_paths      = program_paths.map(out.resolve)
		val programs_folder_exists = in_program_paths.map(Utils.doesFileOrDirectoryExist)
		game.cpus.foreach(cpu => Utils.ensureDirectories(out.resolve("programs_" + cpu.cpuType)))

		relocateTmpSoftware(game, out)

		//		output.Svd(game, out)
		genScripts(game, out, out_program_paths, programs_folder_exists)

	}

	def tmp_program_path(out: Path): Path = out.resolve("tmp")

	private def relocateTmpSoftware(game: Game, out: Path): Unit = {
		Utils.deleteDirectories(out.resolve("libs"))

		// move from the tmp folder to its real place
		Utils.rename(out.resolve("tmp").resolve("libs"),
		             out.resolve("libs"))

		val libraries = game.allSoftwareInstances.collect { case l: LibraryInstance => l }

		val tmpPath = out.resolve("tmp")
		val libPath = out.resolve("libs")

		for (cpu <- game.cpus) {
			val cpuName        = cpu.cpuType
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
			var libraryDefines = ArrayBuffer[String]()

			for (lib <- libraries) {
				val all_cpus = if (lib.attributes.contains("cpus")) {
					val cpusString = Utils.toString(lib.attributes("cpus"))
					if (cpusString == "_") None
					else Some(cpuRegEx.split(cpusString).toSeq)
				} else None

				if (all_cpus.isEmpty || all_cpus.get.contains(cpuName)) {
					val sourcePath = libPath.resolve(lib.name)
					val targetPath = out.resolve(s"libs_$cpuName").resolve(lib.name)
					libraryDefines += lib.name

					Utils.createSymbolicLink(sourcePath, targetPath)
				}
			}

			generateSubdirLibraryMake(out.resolve(s"libs_$cpuName").resolve("CMakeLists.txt"))
			generateCLibraryDefines(out.resolve(s"libs_$cpuName"), libraryDefines.toArray)
			generateCMakeLibraryDefines(out.resolve(s"cmakelibrary_defines_$cpuName.cmake"), libraryDefines.toArray)
		}
	}

	private def generateCLibraryDefines(out: Path, libs: Array[String]): Unit = {
		Utils.ensureDirectories(out.resolve("library_defines/include/library_defines/"))

		val sb = new mutable.StringBuilder
		sb ++= "// Autogenerated!\n"
		sb ++= "#pragma once\n"
		libs.foreach(f => {
			sb ++= s"""#define IKUY_HAVE_LIB_${f.toUpperCase()} 1\n"""
		})
		Utils.writeFile(out.resolve("library_defines/include/library_defines/library_defines.h"), sb.result())
		Utils.copy(Resources.stdResourcePath().resolve("catalogs/software/library_defines.cmake"),
		           out.resolve("library_defines/CMakeLists.txt"))
	}

	private def generateCMakeLibraryDefines(out: Path, libs: Array[String]): Unit = {
		val sb = new mutable.StringBuilder
		sb ++= "# Autogenerated!\n"
		libs.foreach(f => {
			sb ++= s"""set(IKUY_HAVE_LIB_${f.toUpperCase()} 1)\n"""
		})
		Utils.writeFile(out, sb.result())
	}

	private def generateSubdirLibraryMake(out: Path,
	                                      excludes: Seq[String] = Seq()): Unit = {
		val sb = new mutable.StringBuilder
		sb ++=
		"""
			|file(GLOB _all LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			|CONFIGURE_DEPENDS * )
			|list(REMOVE_ITEM _all "CMakeLists.txt" "library_defines.h")
			|""".stripMargin

		for (ed <- excludes) {
			sb ++=
			f"""list(REMOVE_ITEM _all "$ed")
				 |""".stripMargin
		}

		sb ++=
		"""
			|foreach(sd ${_all})
			| add_subdirectory(${sd})
			|endforeach()
			|
			|
			|""".stripMargin

		Utils.writeFile(out, sb.result())
	}

	private def genScripts(game: Game,
	                       out: Path,
	                       programPaths: Seq[Path],
	                       programPathExists: Seq[Boolean]): Unit = {
		val sb = new StringBuilder
		for ((cpu, i) <- game.cpus.zipWithIndex) {
			if (programPathExists(i)) {

				val cpuName     = cpu.cpuType
				val programName = "programs_" + cpuName

				Utils.rename(programPaths(i), out.resolve(programName))

				if (cpuName != "host") {
					val sbpm = new mutable.StringBuilder
					val sbm  = new mutable.StringBuilder
					sbpm ++=
					f"cmake -DCMAKE_TOOLCHAIN_FILE=$$PWD/${cpuName}_toolchain.cmake " +
					f"-G Ninja -S . -B build/$programName -DCPU=$cpuName -DBOARD=${game.board.get.name}%n"

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
						game.cpus.flatMap { cpu => if (cpu.cpuType == "host") None else Some(cpu.triple) }
					} else Seq()

					generateSubdirProgramMake(out.resolve(s"$programName").resolve("CMakeLists.txt"),
					                          excludes)
				}
			}
		}
		Utils.writeFile(out.resolve(s"make_programs.sh"), sb.result())
		Utils.setFileExecutable(out.resolve(s"make_programs.sh"))
		Utils.copy(Resources.stdResourcePath().resolve("catalogs/software/ikuy_root.cmake"),
		           out.resolve("CMakeLists.txt"))
		// workaround for microblaze gcc
		Utils.copy(Resources.stdResourcePath().resolve("catalogs/software/empty-file.ld"),
		           out.resolve("empty-file.ld"))

		Utils.writeFile(out.resolve("make_software.sh"), sb.result())
	}

	private def generateSubdirProgramMake(out: Path,
	                                      excludes: Seq[String] = Seq()): Unit = {
		val sb = new mutable.StringBuilder
		sb ++=
		"""
			|if(DEFINED ONLY_PROGRAM)
			|   add_subdirectory(${ONLY_PROGRAM})
			|else()
			|   file(GLOB _all LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			|   CONFIGURE_DEPENDS * )
			|   list(REMOVE_ITEM _all "CMakeLists.txt" "library_defines.h")
			|""".stripMargin

		for (ed <- excludes) {
			sb ++=
			f"""  list(REMOVE_ITEM _all "$ed")
				 |""".stripMargin
		}

		sb ++=
		"""
			| foreach(sd ${_all})
			|   add_subdirectory(${sd})
			| endforeach()
			|
			|endif()
			|
			|""".stripMargin

		Utils.writeFile(out, sb.result())
	}

}
