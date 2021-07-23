package output

import ikuy_utils.Utils
import overlord.Game
import overlord.Instances.CpuInstance

import java.nio.file.Path

object BootRom {
	def apply(game: Game, out: Path): Unit = {
		if (game.cpus.isEmpty) return

		println(s"Creating BootRom projects at $out")

		val primaryBooters =
			if (game.cpus.length == 1) Seq(game.cpus.head)
			else game.cpus.flatMap { c =>
				if (c.primaryBoot) Some(c)
				else None
			}

		if (primaryBooters.isEmpty) {
			println("There must be at least 1 primary boot cpu if any cpus")
			return
		}

		// todo add boot rom to halt secondary cpus
		val secondaryBooters = game.cpus.diff(primaryBooters)

		genPrimary(primaryBooters, out)
		genShellScript(primaryBooters, secondaryBooters, out)
	}

	private def genPrimary(booters: Seq[CpuInstance], out: Path): Unit = {

		val template = Utils.readFile(
			"CMakeLists",
			Path.of("software/bootrom/CMakeLists.txt"),
			getClass) match {
			case Some(script) => script
			case None         =>
				println("ERROR: resource CMakeLists.txt not found!")
				return
		}

		for (booter <- booters) {
			val triple    = Utils.toString(booter.attributes("triple"))
			val sanTriple = triple.replace("-", "")
			val dir       = out.resolve(sanTriple + "_primary_boot")

			Utils.ensureDirectories(dir)

			Utils.copy(Path.of(s"software/bootrom/primary_boot_$sanTriple.ld"),
			     dir.resolve("boot.ld"))

			Utils.copy(Path.of(s"software/bootrom/crt_$sanTriple.S"),
			     dir.resolve("crt.S"))
			Utils.copy(Path.of(s"software/bootrom/main.c"),
			     dir.resolve("main.c"))

			// try to read a specialist cmake file, if none exist use template
			val tt = Utils.readFile(
				"CMakeLists_" + sanTriple,
				Path.of(s"software/bootrom/CMakeLists_$sanTriple.txt"),
				getClass) match {
				case Some(s) => s
				case None    =>
					template
						.replace("""${boot_cpu}""", sanTriple)
						.replace("""${triple}""", triple)
			}

			Utils.writeFile(dir.resolve("CMakeLists.txt"), tt)
		}
	}

	private def genShellScript(primarys: Seq[CpuInstance],
	                           secondarys: Seq[CpuInstance],
	                           out: Path): Unit = {
		val sb = new StringBuilder

		for (primary <- primarys) {
			val triple    = Utils.toString(primary.attributes("triple"))
			val sanTriple = triple.replace("-", "")
			sb ++=
			"cmake -D " +
			"CMAKE_TOOLCHAIN_FILE=$PWD/" + sanTriple + "_toolchain.cmake " +
			"-G Ninja " +
			"-S bootroms/" + sanTriple + "_primary_boot " +
			"-B build/bootroms/" + sanTriple + "_primary_boot " +
			"\n"

			sb ++= "cmake --build build/bootroms/" + sanTriple + "_primary_boot"
		}
		Utils.writeFile(out.resolve("../make_bootrooms.sh"), sb.result())
	}
}