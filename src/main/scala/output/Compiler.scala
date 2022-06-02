package output

import ikuy_utils.Utils
import overlord.Instances.CpuInstance
import overlord.{Game, Resources}

import java.nio.file.Path
import scala.collection.mutable

object Compiler {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating Compiler scripts at $out")

		game.cpus.foreach(genCompilerScript(_, out))
		game.cpus.foreach(genCMakeToolChains(_, out))
	}

	private def sanatizeTriple(triple: String): String = {
		triple.replace("-", "_")
	}

	private def genCompilerScript(cpu: CpuInstance, out: Path): Unit = {
		if (cpu.host) return

		val (triple, gccFlags) = (cpu.triple, cpu.gccFlags)

		val sb = new mutable.StringBuilder
		sb ++= (Utils.readFile(Resources.stdResourcePath().resolve("catalogs/software/make_compilers.sh")) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource make_compilers.sh not found!")
				return
		})
		sb ++=
		s"""
			 |build_binutils $triple $$PWD/programs_host
			 |build_gcc $triple $$PWD/programs_host "$gccFlags"
			 |""".stripMargin

		Utils.writeFile(out.resolve("make_compilers.sh"), sb.result())
		Utils.setFileExecutable(out.resolve(s"make_compilers.sh"))
	}

	private def genCMakeToolChains(cpu: CpuInstance, out: Path): Unit = {
		if (cpu.host) return

		val (cpuType, triple, gccFlags) = (cpu.cpuType, cpu.triple, cpu.gccFlags)

		val template = Utils.readFile(Resources.stdResourcePath().resolve("catalogs/software/toolchain_template.cmake")) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource make_compilers.sh not found!")
				return
		}

		// try to read a specialist toolchain file, if none exist use template
		val specialist = Resources.stdResourcePath().resolve(s"catalogs/software/toolchain_$cpuType.cmake")
		val tt         = Utils.readFile(specialist) match {
			case Some(s) => s
			case None    => template
				.replace("""${triple}""", triple)
				.replace("""${GCC_FLAGS}""", gccFlags)
		}

		Utils.writeFile(out.resolve(cpuType + "_toolchain.cmake"), tt)
	}
}
