package output

import ikuy_utils.Utils
import overlord.{Game, Resources}

import java.nio.file.Path

object Compiler {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating Compiler scripts at $out")
		Utils.ensureDirectories(out)

		val cpu_info: Set[(String, String, String)] = {
			game.cpus.map(cpu => (
				cpu.definition.defType.ident.last,
				cpu.triple,
				Utils.lookupString(cpu.attributes, "gcc_flags", "")))
		}.toSet

		genCompilerScript(cpu_info, out)
		genCMakeToolChains(cpu_info, out)
	}

	private def sanatizeTriple(triple: String): String = {
		triple.replace("-", "_")
	}

	private def genCompilerScript(cpu_info: Set[(String, String, String)],
	                              out: Path): Unit = {
		val sb = new StringBuilder

		sb ++= (Utils.readFile("generate_compilers",
		                       Resources.stdResourcePath()
			                       .resolve("catalogs/software/generate_compilers.sh"),
		                       getClass) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource generate_compilers.sh not found!")
				return
		})

		cpu_info.foreach { case (_, triple, gcc_flags) =>
			// only build compilers for none os (not hosts)
			if (triple.contains("none")) {
				sb ++=
				s"""
					 |build_binutils $triple $$PWD/compilers
					 |build_gcc $triple $$PWD/compilers "$gcc_flags"
					 |""".stripMargin
			}
		}


		Utils.writeFile(out.resolve("generate_compilers.sh"), sb.result())
		Utils.setFileExecutable(out.resolve(s"generate_compilers.sh"))

	}

	private def genCMakeToolChains(cpu_info: Set[(String, String, String)],
	                               out: Path): Unit = {

		val template = Utils.readFile(
			"toolchain_template",
			Resources.stdResourcePath().resolve("catalogs/software/toolchain_template.cmake"),
			getClass) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource generate_compilers.sh not found!")
				return
		}

		for ((name, triple, gcc_flags) <- cpu_info
		     // only build compilers for none os (not hosts)
		     if triple.contains("none")) {
			// try to read a specialist toolchain file, if none exist use template
			val tt = Utils.readFile(
				name = "toolchain_" + name,
				path = Resources.stdResourcePath().resolve(
					s"catalogs/software/toolchain_$name.cmake"),
				klass = getClass
				) match {
				case Some(s) => s
				case None    => {
					template
						.replace("""${triple}""", triple)
						.replace("""${GCC_FLAGS}""", gcc_flags)
				}
			}

			Utils.writeFile(out.resolve("..").resolve(name + "_toolchain.cmake"), tt)
		}
	}
}
