package output

import ikuy_utils.Utils
import overlord.Game

import java.nio.file.Path

object Compiler {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating Compiler scripts at $out")
		Utils.ensureDirectories(out)

		val triples: Set[String] = {
			game.cpus.map(_.triple)
		}.toSet

		genCompilerScript(triples, out)
		genCMakeToolChains(triples, out)
	}
	private def sanatizeTriple(triple:String) : String = {
		triple.replace("-", "_")
	}

	private def genCompilerScript(triples: Set[String], out: Path): Unit = {
		val sb = new StringBuilder

		sb ++= (Utils.readFile("generate_compilers",
		                       Path.of("software/generate_compilers.sh"),
		                       getClass) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource generate_compilers.sh not found!")
				return
		})

		triples.foreach(triple => sb ++=
		                          s"""
			                           |build_binutils $triple $$PWD/compilers
			                           |build_gcc $triple $$PWD/compilers
			                           |""".stripMargin)


		Utils.writeFile(out.resolve("generate_compilers.sh"), sb.result())
	}

	private def genCMakeToolChains(triples: Set[String], out: Path): Unit = {

		val template = Utils.readFile(
			"toolchain_template",
			Path.of("software/toolchain_template.cmake"),
			getClass) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource generate_compilers.sh not found!")
				return
		}

		for (triple <- triples) {
			val sanTriple = sanatizeTriple(triple)

			// try to read a specialist toolchain file, if none exist use template
			val tt = Utils.readFile(
				name = "toolchain_" + sanTriple,
				path = Path.of(s"software/toolchain_$sanTriple.cmake"),
				klass = getClass
				) match {
				case Some(s) => s
				case None    => template.replace("""${triple}""", triple)
			}

			Utils.writeFile(out.resolve(sanTriple + "_toolchain.cmake"), tt)
		}
	}
}
